CC = gcc
CFLAGS = -Wall -Wextra 

SRCDIR = src
OBJDIR = obj
BINDIR = bin
GLOBAL_BINDIR = /usr/local/bin

# List of source files (assuming one source file per executable)
SOURCES = $(wildcard $(SRCDIR)/*.c)
EXECUTABLES = $(patsubst $(SRCDIR)/%.c,$(BINDIR)/%,$(SOURCES))

all: $(EXECUTABLES) grupo-criar grupo-remover grupo-user-adicionar grupo-user-remover 

$(BINDIR)/%: $(OBJDIR)/%.o | $(BINDIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR)/mail-queue: $(SRCDIR)/mail-queue.c | $(BINDIR)
	$(CC) $(CFLAGS) $< -o $(BINDIR)/mail-queue
	sudo chown root:root $(BINDIR)/mail-queue
	sudo chmod u+s $(BINDIR)/mail-queue

$(BINDIR)/mail-lspawn: $(SRCDIR)/mail-lspawn.c | $(BINDIR)
	$(CC) $(CFLAGS) $< -o $(BINDIR)/mail-lspawn
	sudo chown root:root $(BINDIR)/mail-lspawn
	sudo chmod u+s $(BINDIR)/mail-lspawn

$(BINDIR)/mail-send: $(SRCDIR)/mail-send.c | $(BINDIR)
	$(CC) $(CFLAGS) $< -o $(BINDIR)/mail-send
	sudo chown root:root $(BINDIR)/mail-send
	sudo chmod u+s $(BINDIR)/mail-send

$(BINDIR)/concordia-listar: $(SRCDIR)/concordia-listar.c | $(BINDIR)
	$(CC) $(CFLAGS) $< -o $(BINDIR)/concordia-listar
	sudo chown root:root $(BINDIR)/concordia-listar
	sudo chmod u+s $(BINDIR)/concordia-listar

grupo-criar: $(SRCDIR)/grupo-criar.c | $(BINDIR)
	$(CC) $(CFLAGS) $< -o $(BINDIR)/grupo-criar
	sudo chown root:root $(BINDIR)/grupo-criar
	sudo chmod u+s $(BINDIR)/grupo-criar

grupo-remover: $(SRCDIR)/grupo-remover.c | $(BINDIR)
	$(CC) $(CFLAGS) $< -o $(BINDIR)/grupo-remover
	sudo chown root:root $(BINDIR)/grupo-remover
	sudo chmod u+s $(BINDIR)/grupo-remover

grupo-user-adicionar: $(SRCDIR)/grupo-user-adicionar.c | $(BINDIR)
	$(CC) $(CFLAGS) $< -o $(BINDIR)/grupo-user-adicionar
	sudo chown root:root $(BINDIR)/grupo-user-adicionar
	sudo chmod u+s $(BINDIR)/grupo-user-adicionar

grupo-user-remover: $(SRCDIR)/grupo-user-remover.c | $(BINDIR)
	$(CC) $(CFLAGS) $< -o $(BINDIR)/grupo-user-remover
	sudo chown root:root $(BINDIR)/grupo-user-remover
	sudo chmod u+s $(BINDIR)/grupo-user-remover

estrutura:
	sudo mkdir /home/concordia
	sudo mkdir /home/concordia/queue
	mkfifo /var/tmp/mail_spawn_fifo
	
users_sistema:
	sudo adduser put-queue
	sudo adduser view-queue
	sudo adduser remove-queue

groups_sistema:
	sudo addgroup put-group
	sudo addgroup view-group
	sudo addgroup remove-group

fifo:
	mkfifo /var/tmp/mail_spawn_fifo


permissions:
	sudo rm -r /home/concordia/queue
	sudo mkdir /home/concordia/queue
	sudo chown root:root /home/concordia/queue
	sudo chmod 700 /home/concordia/queue
	sudo setfacl -m g:put-group:wx /home/concordia/queue
	sudo setfacl -m g:view-group:r /home/concordia/queue
	sudo setfacl -m g:remove-group:wx /home/concordia/queue


$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	rm -rf $(OBJDIR) $(BINDIR)