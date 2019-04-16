find ../.. -name '*.lobster' -print \
  -exec ../../bin/lobster.exe --non-interactive-test --silent {} \;
