@if not "x%1x"=="xdoxyx" goto build
@doxygen Doxyfile
@python doxy2swig.py doxy\xml\index.xml cmf\cmf_core_src\docstrings.i
@ren cmf\cmf_core_src\cmf.i cmf_1.i
@ren cmf\cmf_core_src\cmf_1.i cmf.i
:build
@if "x%1x"=="xcleanx" goto clean
@python setup.py build_ext swig
@if ERRORLEVEL 1 goto error
@move /Y cmf\cmf_core_src\cmf_core.py cmf\
@move /Y cmf\raster\raster_src\raster.py cmf\raster\
@python setup.py build_py -c -O2 -f
@if ERRORLEVEL 1 goto error
@python setup.py install
@if ERRORLEVEL 1 goto error
@goto end
:clean
@del build /s /q
@goto end
:error
@echo ************************
@echo Installation had error !
@echo ************************
:end
@echo .