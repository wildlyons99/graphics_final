ASSIGN     = final
BREWPATH   = $(shell brew --prefix)
CXX        = $(shell fltk-config --cxx)
CXXFLAGS   = $(shell fltk-config --cxxflags) -I$(BREWPATH)/include
LDFLAGS    = $(shell fltk-config --ldflags --use-gl --use-images) -L$(BREWPATH)/lib
POSTBUILD  = fltk-config --post #build .app for osx. (does nothing on pc)

$(ASSIGN): % : main.o MyGLCanvas.o ppm.o ply.o ShaderManager.o ShaderProgram.o TextureManager.o
	$(CXX) $(LDFLAGS) $^ -o $@
	$(POSTBUILD) $@
	
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

clean:
	rm -rf $(ASSIGN) $(ASSIGN).app *.o *~ *.dSYM
