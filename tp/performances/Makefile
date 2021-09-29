%: %.c
	$(CC) -Wall -Wextra -o $@ $<

exec_time.dat: random_data.dat cat experience.sh
	./experience.sh $< > $@

random_data.dat:
	dd if=/dev/urandom of=$@ bs=1M count=100

plot.svg: exec_time.dat plot.gnuplot
	gnuplot plot.gnuplot

clean:
	rm -rf cat cat-lib random_data.dat exec_time.dat plot.svg

.DEFAULT: plot.svg
.PHONY: clean
