cd ..
bin\lobster --wait --gen-builtins-html modules\std.lobster
rem bin\lobster --wait --gen-builtins-names modules\std.lobster
rem FIXME
move modules\builtin_functions_reference.html docs\
cd docs
