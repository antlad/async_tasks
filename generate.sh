#/bin/bash
for i in {1..10}; do dd if=/dev/urandom bs=1048576 count=100 of=file$i.raw; done