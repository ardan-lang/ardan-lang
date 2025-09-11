#!/bin/bash

INTERPRETER=./tau-programming-lang
TEST_DIR=tests
TMP_OUT=.test_tmp_out
PASS=0
FAIL=0

for testfile in "$TEST_DIR"/*.ardan; do
    testname=$(basename "$testfile" .ardan)
    EXPECTED="$TEST_DIR/$testname.out"

    echo "===== Running $testfile ====="
    $INTERPRETER --i "$testfile" > "$TMP_OUT" 2>&1
    STATUS=$?
    cat "$TMP_OUT"

    if [ -f "$EXPECTED" ]; then
        # There is a golden output file to compare
        if diff -q "$TMP_OUT" "$EXPECTED" >/dev/null; then
            echo "PASS: $testfile (output matches expected)"
            let PASS++
        else
            echo "FAIL: $testfile (output differs from $EXPECTED)"
            let FAIL++
            echo "--- Expected ---"
            cat "$EXPECTED"
            echo "--- Actual ---"
            cat "$TMP_OUT"
        fi
    elif [ $STATUS -eq 0 ]; then
        echo "PASS: $testfile (no golden output, ran successfully)"
        let PASS++
    else
        echo "FAIL: $testfile (runtime error)"
        let FAIL++
    fi
    echo
done

rm -f "$TMP_OUT"

echo "TOTAL: $PASS passed, $FAIL failed."
exit $FAIL