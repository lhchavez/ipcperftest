# ipcperftest

Small tests for different IPC mechanisms:

* `pipetest`: Named pipes. Serializes an array, deserializes it in another process.
* `shmtest`: Shared memory w/semaphores.
* `shmpipetest`: Shared memory w/named pipes instead of semaphores.
* `transacttest`: Shared memory w/transact[1].

[1]: https://github.com/lhchavez/transact
