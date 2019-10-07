typedef struct Mutex Mutex;

Mutex *mutex_create(void);

void mutex_lock(Mutex *mutex);

void mutex_unlock(Mutex *mutex);
