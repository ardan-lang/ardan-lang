How to run:
1. Save as run_all_tests.sh in your project root.
2. Run: chmod +x run_all_tests.sh
3. Execute: ./run_all_tests.sh

How to run:
1. Save above as run_all_tests.sh and chmod +x run_all_tests.sh
2. To add golden tests, create tests/NAME.out with the expected output for any tests/NAME.ardan
3. Run with ./run_all_tests.sh

# XCode

To avoid "../.."

1. Select project
2. Build Settings
3. Search: Header Search Paths
4. Add:

```
$(SRCROOT)/src
```