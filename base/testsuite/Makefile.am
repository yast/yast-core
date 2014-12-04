ITESTS = \
  target_root.ycp

EXTRA_DIST = $(ITESTS) y2base.sh

check-local:
	for t in $(ITESTS); do \
	  ./y2base.sh $$t || { cat $${t%.*}.log; exit 1; }; \
	done
