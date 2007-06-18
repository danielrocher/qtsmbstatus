#include "pam.h"

// most of this code I got from Rene Mayrhofer (rmayr@debian.org)
  
/* Global variables for PAM authentication. */
static char *pamUsername, *pamPassword;

/* This is the PAM conversation function, it uses the global variables
 pamUsername and pamPassword, they have to be initialized before using this
 function.
 It simply feeds the password to the PAM library in response to a
 PAM_PROMPT_ECHO_OFF message */
static int pamConversationFunction(int num_msg, const struct pam_message **msg,
				   struct pam_response **resp, void *appdata_ptr) {
    struct pam_response *r;
    int count;

    // alloc the response
    r = (struct pam_response*) malloc(sizeof(struct pam_response) * num_msg);
    if (r == NULL)
	return PAM_CONV_ERR;

    for (count=0; count<num_msg; count++) {
	switch ((*msg)[count].msg_style) {
	case PAM_PROMPT_ECHO_ON:
	    r[count].resp = (char*) malloc(PAM_MAX_RESP_SIZE);
	    strncpy(r->resp, pamUsername, PAM_MAX_RESP_SIZE);
	    break;

	case PAM_PROMPT_ECHO_OFF:
	    r[count].resp = (char*) malloc(PAM_MAX_RESP_SIZE);
	    strncpy(r[count].resp, pamPassword, PAM_MAX_RESP_SIZE);
	    r[count].resp_retcode = PAM_SUCCESS;
	    break;
	default:
	    free(r);
	    return PAM_CONV_ERR;
	}
    }
    *resp = r;

    return PAM_SUCCESS;
}

/* pamUsername and pamPassword must be set before calling this method.
 Returns 0 when not successful, 1 when successful; */
int checkUserPass_real() {
    struct pam_conv pam_conversation;
    pam_handle_t *pam_h;
    int pamretval, ret=0;

    pam_conversation.conv = pamConversationFunction;
    pam_conversation.appdata_ptr = NULL;
    pamretval = pam_start(PAM_SERVICE_NAME, pamUsername, &pam_conversation, &pam_h);
    if (pamretval != PAM_SUCCESS) {
	//printf("Error initializing PAM library: %s\n", pam_strerror(pam_h, pamretval));
	return 0;
    }

    pamretval = pam_authenticate(pam_h, PAM_SILENT);
    if (pamretval != PAM_SUCCESS) {
	//printf("User could not be authenticated: %s\n", pam_strerror(pam_h, pamretval));
	ret = 0;
    }
    else {
	pamretval = pam_acct_mgmt(pam_h, 0);
	if (pamretval != PAM_SUCCESS) {
//	    printf("User not healthy: %s\n", pam_strerror(pam_h, pamretval));
	    ret = 0;
	}
	else
	    ret = 1;
    }

    if (pam_end(pam_h, pamretval) != PAM_SUCCESS) {
//	printf("Error releasing PAM library: %s\n", pam_strerror(pam_h, pamretval));
	return 0;
    }
    return ret;
}

/* authenticate vs pam. Notice: PAM_SERVICE_NAME: pam service has to exist and be set up correctly*/
int auth(char *username, char *passwd) {
    if (!username)
        return 0;
    if (!passwd)
        return 0;

    pamUsername = strdup(username);
    pamPassword = strdup(passwd);
    if (checkUserPass_real())
        return 1;
    else
        return 0;
}

