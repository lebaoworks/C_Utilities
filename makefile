CC = gcc

# Compile flags
CFLAGS = -Wall

# Library flags
LDFLAGS = 

TARGET = main
SRCDIR = util
OBJDIR = obj
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))


all: $(OBJECTS)
	$(CC) $(CFLAGS) $(TARGET).c $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -I$(SRCDIR) -c $< -o $@

$(OBJECTS) : | $(OBJDIR)
$(OBJDIR):
	mkdir -p $(OBJDIR)

debug: clean $(eval CFLAGS += -g) all
	
clean:
	rm -rf $(OBJDIR) $(TARGET)
