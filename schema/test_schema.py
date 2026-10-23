"""Every examples/valid_*.json must validate, every examples/invalid_*.json must be rejected. Run: python3 -m unittest"""
import json
import unittest
from pathlib import Path

import oaax_manifest

EXAMPLES = sorted((Path(__file__).parent / "examples").glob("*.json"))


class ManifestExamples(unittest.TestCase):
    def test_examples(self):
        for path in EXAMPLES:
            with self.subTest(path.name):
                problems = oaax_manifest.validate(json.loads(path.read_text()))
                if path.name.startswith("valid_"):
                    self.assertEqual(problems, [])
                else:
                    self.assertTrue(problems, "expected the manifest to be rejected")

    def test_model_file_must_exist(self):
        doc = json.loads((EXAMPLES[0].parent / "valid_identity_cpu.json").read_text())
        self.assertTrue(oaax_manifest.validate(doc, str(EXAMPLES[0].parent)))  # no model.onnx in examples/


if __name__ == "__main__":
    unittest.main()
