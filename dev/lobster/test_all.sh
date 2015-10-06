find ../.. -name '*.lobster' -print \
  -exec ../../lobster/lobster.exe --non-interactive-test --silent {} \;
