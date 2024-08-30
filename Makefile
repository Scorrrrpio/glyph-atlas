CXX = g++
CXXFLAGS = -I/usr/include/freetype2
LDFLAGS = -lfreetype

exec: main.o
	$(CXX) -o $@ $^ $(LDFLAGS)

main.o: src/main.cpp
	$(CXX) $(CXXFLAGS) -c src/main.cpp

clean:
	rm -f *.o exec