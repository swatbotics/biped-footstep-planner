PROGFILES = bipedPlan

all: $(PROGFILES)

INCLUDES = -Iinclude -I/opt/local/include

LIBS = -lpng -L/opt/local/lib

%.o: %.cpp include/*.h 
	g++ -O3 -Wall $(INCLUDES) -c $< -o $@

bipedPlan: src/biped.o src/biped_checker.o src/bipedSearch.o src/bipedPlan.o
	g++ -O3 -Wall -o $@ $^ $(LIBS)

clean: 
	rm -f src/*.o biped_test src/*~ include/*~ *~

.PHONY: clean
