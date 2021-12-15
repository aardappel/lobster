cd ..
bin\lobster --wait --gen-builtins-html
rem bin\lobster --wait --gen-builtins-names
rem FIXME
move builtin_functions_reference.html docs\
cd docs
