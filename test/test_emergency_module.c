#include "../src/emergency_module.h"
#include <stdatomic.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>

EmergencyNode_t* initialize_node() {
    EmergencyNode_class_init();
    EmergencyNode_t* emergencyNode = malloc(sizeof(EmergencyNode_t));
    EmergencyNode_init(emergencyNode);

    return emergencyNode;
}

void test_class_init() {
    int8_t result = EmergencyNode_class_init();
    assert(result == 0);

    // Init already done
    result = EmergencyNode_class_init();
    assert(result == -1);
}

void test_init() {
    EmergencyNode_t* emergencyNode = initialize_node();

    assert(sizeof(*emergencyNode) == sizeof(EmergencyNode_t));
    emergencyNode->emergency_counter = 50;
    emergencyNode->emergency_buffer[0] = 2;

    int8_t result = EmergencyNode_init(emergencyNode);
    assert(result == 0);
    EmergencyNode_t zero = {0}; // Node with all zeros
    assert(memcmp(emergencyNode, &zero, sizeof(EmergencyNode_t)) == 0);

    /* Or
    for (int i = 0; i < NUM_EMERGENCY_BUFFER; i++) {
        assert(emergencyNode->emergency_buffer[i] == 0);

    }
    assert(emergencyNode->emergency_counter == 0);
    */

    free(emergencyNode);
}

void test_raise_valid() {
    EmergencyNode_t* emergencyNode = initialize_node();

    // true, true, counter increased
    int8_t result = EmergencyNode_raise(emergencyNode, 3);
    assert(result == 0);
    assert(emergencyNode->emergency_counter == 1);

    // false, _, same exception, counter already increased
    result = EmergencyNode_raise(emergencyNode, 3);
    assert(result == 0);
    assert(emergencyNode->emergency_counter == 1);

    // true, true
    result = EmergencyNode_raise(emergencyNode, 4);
    assert(result == 0);
    assert(emergencyNode->emergency_counter == 2);

    // Solve the exceptions to make the global counter go back to 1
    EmergencyNode_solve(emergencyNode, 3);
    EmergencyNode_solve(emergencyNode, 4);
    free(emergencyNode);
}

void test_raise_invalid() {
    EmergencyNode_t* emergencyNode = initialize_node();

    int8_t result = EmergencyNode_raise(emergencyNode, NUM_EMERGENCY_BUFFER*8);
    assert(result == -1);
    assert(emergencyNode->emergency_counter == 0);

    free(emergencyNode);
}

void test_solve_valid() {
    EmergencyNode_t* emergencyNode = initialize_node();

    // Try to solve an emergency
    int8_t result = EmergencyNode_solve(emergencyNode, 3);
    assert(result == 0);
    assert(emergencyNode->emergency_counter == 0);

    // First raise some emergencies and then solve them
    EmergencyNode_raise(emergencyNode, 3);
    EmergencyNode_raise(emergencyNode, 5);

    // Solve an emergency
    result = EmergencyNode_solve(emergencyNode, 3);
    assert(result == 0);
    assert(emergencyNode->emergency_counter == 1);

    // Nonexistent emergency
    result = EmergencyNode_solve(emergencyNode, 4);
    assert(result == 0);
    assert(emergencyNode->emergency_counter == 1);

    // Solve the last exception to make the global counter go back to 0
    result = EmergencyNode_solve(emergencyNode, 5);
    assert(result == 0);
    assert(emergencyNode->emergency_counter == 0);

    free(emergencyNode);
}

void test_solve_invalid() {
    EmergencyNode_t* emergencyNode = initialize_node();

    int8_t result = EmergencyNode_solve(emergencyNode, NUM_EMERGENCY_BUFFER*8);
    assert(result == -1);

    free(emergencyNode);
}

void test_is_emergency_state() {
    EmergencyNode_t* emergencyNode = initialize_node();

    // No emergency raised
    int8_t result = EmergencyNode_is_emergency_state(emergencyNode);
    assert(result == 0);

    // First raise an emergency
    EmergencyNode_raise(emergencyNode, 3);

    result = EmergencyNode_is_emergency_state(emergencyNode);
    assert(result == 1);

    // Solve the emergency
    EmergencyNode_solve(emergencyNode, 3);

    result = EmergencyNode_is_emergency_state(emergencyNode);
    assert(result == 0);

    free(emergencyNode);
}

void test_destroy() {
    EmergencyNode_t* emergencyNode = initialize_node();

    assert(sizeof(*emergencyNode) == sizeof(EmergencyNode_t));
    emergencyNode->emergency_counter = 50;
    emergencyNode->emergency_buffer[0] = 2;

    int8_t result = EmergencyNode_destroy(emergencyNode);
    assert(result == 0);
    EmergencyNode_t zero = {0}; // Node with all zeros
    assert(memcmp(emergencyNode, &zero, sizeof(EmergencyNode_t)) == 0);

    /* Or
    for (int i = 0; i < NUM_EMERGENCY_BUFFER; i++) {
        assert(emergencyNode->emergency_buffer[i] == 0);

    }
    assert(emergencyNode->emergency_counter == 0);
    */

    free(emergencyNode);
}

void* raise_emergency(void* arg) {
    EmergencyNode_t* emergencyNode = (EmergencyNode_t*)arg;
    for (int i = 0; i < 1000; i++) {
        EmergencyNode_raise(emergencyNode, i);
    }
    return NULL;
}

void* solve_emergency(void* arg) {
    EmergencyNode_t* emergencyNode = (EmergencyNode_t*)arg;
    for (int i = 0; i < 1000; i++) {
        EmergencyNode_solve(emergencyNode, i);
    }
    return NULL;
}

void test_thread() {
    // Thread to raise and solve emergencies
    pthread_t raise_emergency_thread;
    pthread_t solve_emergency_thread;

    // Create the nodes and pass them by argument
    EmergencyNode_t* raiseNode = initialize_node();
    pthread_create(&raise_emergency_thread, NULL, raise_emergency, raiseNode);

    EmergencyNode_t* solveNode = initialize_node();
    pthread_create(&solve_emergency_thread, NULL, solve_emergency, solveNode);

    pthread_join(raise_emergency_thread, NULL);
    pthread_join(solve_emergency_thread, NULL);

    // Conformity check on emergency states
    int8_t raiseNode_emergency_state = EmergencyNode_is_emergency_state(raiseNode);
    int8_t solveNode_emergency_state = EmergencyNode_is_emergency_state(raiseNode);
    assert(raiseNode_emergency_state == solveNode_emergency_state);

    // Conformity check on emergency_counter and emergency_state
    uint32_t emergency_counter = raiseNode->emergency_counter + solveNode->emergency_counter;
    if (raiseNode_emergency_state == 0) {
        assert(emergency_counter == 0);
    } else {
        assert(emergency_counter > 0);
    }

    free(raiseNode);
    free(solveNode);
}

int main() {
    test_class_init();
    test_init();
    test_raise_valid();
    test_raise_invalid();
    test_solve_valid();
    test_solve_invalid();
    test_is_emergency_state();
    test_destroy();
    test_thread();

    return 0;
}