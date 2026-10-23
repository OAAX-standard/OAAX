"""Derive and validate an OAAX bundle manifest (oaax-manifest.json).

  derive_io(onnx_path)    -> inputs/outputs for a model whose runtime takes the ONNX tensors as they are
  validate(doc, dir=None) -> list of problems (empty = valid); with `dir`, also checks that model_file exists
"""
import json
import sys
from pathlib import Path

import jsonschema

SCHEMA = json.loads((Path(__file__).parent / "oaax-manifest.schema.json").read_text())
FLOATS = {"FLOAT", "FLOAT16", "BFLOAT16", "DOUBLE"}
UNIQUE_AXES = "NCHWDT"  # X (no meaning) may repeat


def _dtype_name(elem_type: int) -> str:
    from onnx import TensorProto

    return TensorProto.DataType.Name(elem_type)


def _layout(rank: int, dtype: str) -> str:
    # ONNX carries no layout: images (rank 4) are NCHW by convention, everything else has no known axes
    return "NCHW" if rank == 4 and dtype in FLOATS | {"UINT8"} else "X" * rank


def _tensor(value_info) -> dict:
    tt = value_info.type.tensor_type
    shape = [d.dim_value if d.HasField("dim_value") and d.dim_value > 0 else -1 for d in tt.shape.dim]
    view = {"dtype": _dtype_name(tt.elem_type), "shape": shape, "layout": _layout(len(shape), _dtype_name(tt.elem_type))}
    return {"name": value_info.name, "onnx": view, "native": dict(view)}


def derive_io(onnx_path: str) -> dict:
    import onnx

    graph = onnx.load(onnx_path, load_external_data=False).graph
    initializers = {i.name for i in graph.initializer}
    return {
        "inputs": [_tensor(i) for i in graph.input if i.name not in initializers],
        "outputs": [_tensor(o) for o in graph.output],
    }


def _check_view(where: str, view: dict) -> list[str]:
    problems = []
    if len(view["layout"]) != len(view["shape"]):
        problems.append(f"{where}: layout '{view['layout']}' has {len(view['layout'])} letters for rank {len(view['shape'])}")
    for axis in UNIQUE_AXES:
        if view["layout"].count(axis) > 1:
            problems.append(f"{where}: axis {axis} appears twice in layout '{view['layout']}'")
    return problems


def _permuted(shape: list[int], src: str, dst: str) -> list[int] | None:
    """Shape of `src` laid out as `dst` (letters matched; X axes keep their order); None if not a permutation."""
    if sorted(src) != sorted(dst):
        return None
    xs = iter(d for d, a in zip(shape, src) if a == "X")
    by_axis = {a: d for d, a in zip(shape, src) if a != "X"}
    return [next(xs) if a == "X" else by_axis[a] for a in dst]


def _check_pair(where: str, onnx: dict, native: dict) -> list[str]:
    problems = []
    if _permuted(onnx["shape"], onnx["layout"], native["layout"]) != native["shape"]:
        problems.append(f"{where}: native shape {native['shape']} ({native['layout']}) is not the ONNX shape "
                        f"{onnx['shape']} ({onnx['layout']}) with its axes permuted")
    src, dst, has_q = onnx["dtype"], native["dtype"], "quantization" in native
    if src in FLOATS and dst not in FLOATS:
        if not has_q:
            problems.append(f"{where}: {src} -> {dst} needs quantization")
    elif has_q:
        problems.append(f"{where}: quantization only applies to a float tensor stored as integers")
    elif src != dst and src not in FLOATS:
        problems.append(f"{where}: {src} -> {dst} is not a mechanical conversion")
    return problems


def validate(doc: dict, dir: str | None = None) -> list[str]:
    problems = [f"{'/'.join(map(str, e.absolute_path))}: {e.message}" for e in jsonschema.Draft202012Validator(SCHEMA).iter_errors(doc)]
    if problems:
        return problems
    for kind in ("inputs", "outputs"):
        names = [t["name"] for t in doc[kind]]
        problems += [f"{kind}: duplicate name '{n}'" for n in sorted({n for n in names if names.count(n) > 1})]
        for t in doc[kind]:
            where = f"{kind}[{t['name']}]"
            problems += _check_view(f"{where}.native", t["native"])
            if "onnx" in t:
                problems += _check_view(f"{where}.onnx", t["onnx"])
                problems += _check_pair(where, t["onnx"], t["native"])
    if dir is not None and not (Path(dir) / doc["model_file"]).is_file():
        problems.append(f"model_file '{doc['model_file']}' not found in {dir}")
    return problems


if __name__ == "__main__":
    path = Path(sys.argv[1])
    found = validate(json.loads(path.read_text()), str(path.parent) if "--dir" in sys.argv else None)
    print("\n".join(found) or "OK")
    sys.exit(1 if found else 0)
