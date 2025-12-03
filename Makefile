.PHONY: all clean common server user

all: common server user

common:
	$(MAKE) -C common

server:
	$(MAKE) -C server

user:
	$(MAKE) -C user

clean:
	$(MAKE) -C common clean
	$(MAKE) -C server clean
	$(MAKE) -C user clean
