struct Path {
	
	char* name;
	struct Path* previous;
	struct Path* next;
	
};
struct Path;
typedef struct Path Path;

Path* Path_create(char* name);
void Path_destroy(Path* self);

char* Path_to_string(Path* self);