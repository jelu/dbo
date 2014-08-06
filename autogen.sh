#!/bin/sh

echo "Running autoreconf" &&
rm -f config.guess \
	config.sub \
	install-sh \
	ltmain.sh \
	m4/libtool.m4 \
	m4/ltoptions.m4 \
	m4/ltsugar.m4 \
	m4/ltversion.m4 \
	m4/lt~obsolete.m4 &&
autoreconf --install
