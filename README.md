# Lodis

This is a local dictionary service

# How to use

  1. include header file lodis_api.h.
  2. fisrtly , call lodis_init to initialize memory.
  3. use lodis_set , lodis_get , lodis_del to manipulate with keys.
  4. use lodis_logon and lodis_logoff to turn on/off log pring.
  5. use log_flushAll to flush all keys.
  6. call lodis_close to free meemory before process exit.
