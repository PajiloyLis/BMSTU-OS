/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _PROD_CONS_H_RPCGEN
#define _PROD_CONS_H_RPCGEN

#include <rpc/rpc.h>

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PROD 0
#define CONS 1

#define BUFFER_SIZE 4096

#define SEMAPHORE_CNT 3
//semaphore nums
#define BINARY_SEM 0
#define BUFFER_EMPTY 1
#define BUFFER_FULL 2

struct prod_cons {
	char type;
	char result;
};
typedef struct prod_cons prod_cons;

#define producer_consumer_prog 0x20000002
#define producer_consumer_ver 1

#if defined(__STDC__) || defined(__cplusplus)
#define service 1
extern  enum clnt_stat service_1(int *, char *, CLIENT *);
extern  bool_t service_1_svc(int *, char *, struct svc_req *);
extern int producer_consumer_prog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define service 1
extern  enum clnt_stat service_1();
extern  bool_t service_1_svc();
extern int producer_consumer_prog_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_prod_cons (XDR *, prod_cons*);

#else /* K&R C */
extern bool_t xdr_prod_cons ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_PROD_CONS_H_RPCGEN */
