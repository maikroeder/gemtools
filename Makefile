
all:
	$(MAKE) -C GEMTools
	python setup.py build

gemtools:
	$(MAKE) -C GEMTools release

install: all
	python setup.py install

install-user: all
	python setup.py install --user

test: all
	-$(MAKE) -C GEMTools check
	python setup.py nosetests

test-c: all
	-$(MAKE) -C GEMTools check

test-python: all
	python setup.py nosetests --with-xunit

package:
	python setup.py package

clean:
	$(MAKE) -C GEMTools clean
	@rm -Rf build dist
	@rm -Rf python/Gem.egg-info
	@rm -Rf python/Gemtools.egg-info


