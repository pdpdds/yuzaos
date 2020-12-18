struct Terminal;
struct Command;

typedef struct {
	
	char* name;
	void (*execute)(struct Terminal* terminal, struct Command* self, char* arguments);
	
} Command;

Command* Command_create(char* name, void (*execute)(struct Terminal* terminal, struct Command* self, char* arguments));