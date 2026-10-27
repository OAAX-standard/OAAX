"""Turn the old, unversioned pages at the root of a gh-pages checkout into redirects.

Before versioning, the site lived at the root (docs.oaax.org/Specification/). mike leaves those
pages in place, so they would go stale forever. Run this once, after the first `latest` deploy:

    python3 .github/scripts/legacy_redirects.py <gh-pages checkout> docs

Only pages that exist both in docs/ and at the root of the checkout are rewritten.
"""
import sys
from pathlib import Path
from urllib.parse import quote

STUB = """<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>Redirecting</title>
  <link rel="canonical" href="{url}" />
  <noscript><meta http-equiv="refresh" content="1; url={url}" /></noscript>
  <script>window.location.replace("{url}" + window.location.search + window.location.hash);</script>
</head>
<body>This page moved to <a href="{url}">{url}</a>.</body>
</html>
"""


def main(site: Path, docs: Path) -> int:
    done = 0
    for page in sorted(docs.glob("*.md")):
        if page.stem == "index":
            continue
        old = site / page.stem / "index.html"
        if not old.is_file():
            continue
        # Absolute, so it works whether or not the old URL had a trailing slash.
        old.write_text(STUB.format(url=f"/latest/{quote(page.stem)}/"))
        print(f"{page.stem}/ -> /latest/{quote(page.stem)}/")
        done += 1
    return 0 if done else 1


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    sys.exit(main(Path(sys.argv[1]), Path(sys.argv[2])))
