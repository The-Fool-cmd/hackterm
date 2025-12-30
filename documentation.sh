#!/bin/sh

# Generate docs
echo "Generating documentation..."
doxygen Doxyfile || { echo "generation failed"; exit 1;}

# Serve docs
DOCS_DIR=docs/html

if [! -d "$DOCS_DIR"]; then
    echo "Error: '$DOCS_DIR' not found. Did generation fail?"
    exit 1
fi

echo "Serving documentation at http://localhost:8000 ..."
cd "$DOCS_DIR" || exit 1
python3 -m http.server 8000

