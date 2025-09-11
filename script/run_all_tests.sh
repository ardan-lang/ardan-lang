#!/bin/bash

INTERPRETER=./tau-programming-lang
TEST_DIR=tests
PASS=0
FAIL=0

for testfile in "$TEST_DIR"/*.ardan; do
    echo "===== Running $testfile ====="
    OUTPUT=$($INTERPRETER --i "$testfile" 2>&1)
    echo "$OUTPUT"
    # Optionally: Add simple output validation by grepping for known pass/fail strings.
    # For now, count as pass if the interpreter does NOT exit with error.
    if [ $? -eq 0 ]; then
        echo "PASS: $testfile"
        let PASS++
    else
        echo "FAIL: $testfile"
        let FAIL++
    fi
    echo
done

echo "TOTAL: $PASS passed, $FAIL failed."
exit $FAIL