#ifndef LOSETUPIO__H
#define LOSETUPIO__H
#ifdef __cplusplus
extern "C" {
#endif
int   xset_loop (const char 	*device,
		 const char 	*file,
		 int 		offset,
		 const char 	*encryption,
		 int 		*loopro,
		 const char 	*c_passwd);
#ifdef __cplusplus
}
#endif
#endif
