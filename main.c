#include <stdlib.h>
#define MAX_STACK_SIZE 250
#define GC_THRESHOLD 10

typedef enum ObjectType {
    OBJ_INT,
    OBJ_PAIR,
} ObjectType;

typedef struct Object {
    unsigned char mark;
    struct Object* next_object;
    ObjectType type;
    union {
        // OBJ_INT
        int value;

        // OBJ_PAIR
        struct {
            struct Object* head;
            struct Object* tail;
        };
    };
} Object;

typedef struct VM {
    Object* first_object;
    Object* stack[MAX_STACK_SIZE];
    int stack_count;
    int num_objects;
    int max_objects;
} VM;

VM* new_vm() {
    VM* vm = malloc(sizeof(VM));
    vm->stack_count = 0;
    vm->first_object = NULL;
    vm->num_objects = 0;
    vm->max_objects = GC_THRESHOLD;
    return vm;
}

Object* new_object(VM* vm, ObjectType type) {
    if (vm->num_objects==vm->max_objects) gc(vm);
    Object* object = malloc(sizeof(Object));
    object->type = type;
    object->mark = 0;
    object->next_object = vm->first_object;
    vm->first_object = object;
    vm->num_objects++;
    return object;
}

void push(VM* vm, Object* object) {
    assert(vm->stack_count<MAX_STACK_SIZE, "Stack Overflow!");
    vm->stack[vm->stack_count++] = object;
}

void push_int(VM* vm, int value) {
    Object* object = new_object(vm, OBJ_INT);
    object->value = value;
    push(vm, object);
}

void push_pair(VM* vm) {
    Object* object = new_object(vm, OBJ_PAIR);
    object->head = pop(vm);
    object->tail = pop(vm);
    push(vm, object);
}

Object* pop(VM* vm) {
    assert(vm->stack_count>0, "Stack Underflow!");
    return vm->stack[--vm->stack_count];
}

void mark(Object* object) {
    if (object->mark) return;
    object->mark = 1;
    if (object->type==OBJ_PAIR) {
        mark(object->head);
        mark(object->tail);
    }
}

void mark_all(VM* vm) {
    for (int i=0; i<vm->stack_count; i++) {
        mark(vm->stack[i]);
    }
}

void sweep(VM* vm) {
    Object** p_object = &vm->first_object;
    while (*p_object) {
        if ((*p_object)->mark==0) {
            Object* unreached = *p_object;
            *p_object = unreached->next_object;
            free(unreached);
        } else {
            (*p_object)->mark = 0;
            p_object = &(*p_object)->next_object;
        }
    }
}

void gc(VM* vm) {
    mark_all(vm);
    sweep(vm);
    vm->max_objects = vm->num_objects*2;
}