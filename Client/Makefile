# Compiler and Linker
CXX = \
  g++
  
# The Target Binary Program
TARGET_NAME = \
  CNP_Client
  
# Extra flags to give to the C++ compiler.
CXXFLAGS = \
  -Wall -I$(INCLUDE_DIR) -std=c++17

# Include directory
INCLUDE_DIR = \
  ../Include

# Intermediate object directory
OBJ_DIR = \
  ../Obj/Server

DEPENDS_DIR = \
  ../Obj

DEPENDS_FILE = \
  $(DEPENDS_DIR)/$(*F)

# Executable output directory
OUTPUT_DIR = \
  ../Bin

# Compilable objects, prefixed with OBJ_DIR
OBJECTS =  \
  $(addprefix $(OBJ_DIR)/, CNP_Client.o CNP_Socket.o )

DEPENDS =  \
  ${OBJECTS:.o=.d}

LINK_TARGET =  \
  $(addprefix $(OUTPUT_DIR)/, $(TARGET_NAME) )
  
REBUILDABLES = \
  $(OBJECTS) $(DEPENDS) $(LINK_TARGET)

all: $(OBJ_DIR) $(OUTPUT_DIR) $(LINK_TARGET)
	@echo All done

# Pull in dependency info
-include $(DEPENDS)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

.PHONY: clean

clean:
	rm -f $(REBUILDABLES)
	@echo Clean done

rebuild: clean all

# Link the object files
$(LINK_TARGET): $(OBJECTS)
	$(CXX) -g -o $@ $^ $(CXXFLAGS)

# compile and generate dependency info;
# more complicated dependency computation, so all prereqs listed
# will also become command-less, prereq-less targets
#   sed:    strip the target (everything before colon)
#   sed:    remove any continuation backslashes
#   fmt -1: list words one per line
#   sed:    strip leading spaces
#   sed:    add trailing colon
$(OBJ_DIR)/%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)
	@$(CXX) -MM $(CXXFLAGS) $*.cpp > $(DEPENDS_FILE).d
	@mv -f $(DEPENDS_FILE).d $(DEPENDS_FILE).d.tmp
	@sed -e 's|.*:|$*.o:|' < $(DEPENDS_FILE).d.tmp > $(DEPENDS_FILE).d
	@sed -e 's/.*://' -e 's/\\$$//' < $(DEPENDS_FILE).d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $(DEPENDS_FILE).d
	@rm -f $(DEPENDS_FILE).d.tmp
