#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
extern "C" {
  #include <security/pam_appl.h>
  #include <security/pam_misc.h>
}
#define PAM_SERVICE_NAME "qtsmbstatusd"

int auth(char *username,char* passwd);

