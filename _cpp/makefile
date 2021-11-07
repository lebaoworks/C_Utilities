CC = g++

# Compile flags
CFLAGS = -Wall -dM -std=c++11

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

com: clean $(eval CFLAGS += -g) $(OBJECTS)
	$(CC) $(CFLAGS) server.cpp $(OBJECTS) -o server $(LDFLAGS)
	$(CC) $(CFLAGS) client.cpp $(OBJECTS) -o client $(LDFLAGS)
	
clean:
	rm -rf $(OBJDIR) $(TARGET) server client uds_server
