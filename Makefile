abstractor.out: abstractor.cpp Monitor.h Monitor.cpp
	g++ -pthread abstractor.cpp Monitor.h Monitor.cpp -o abstractor.out

clean:
	rm abstractor.out output_file_name.txt
