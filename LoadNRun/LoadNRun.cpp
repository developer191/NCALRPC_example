// LoadNRun.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "ClientStubLibrary_h.h" // header file generated by MIDL compiler

#if defined(_UNICODE) || defined(UNICODE)
#define CHAR_TYPE wchar_t
#define RPC_STR RPC_WSTR
#define FORMAT_STORE_CHAR _snwprintf_s
#define FORMAT_PRINT wprintf_s
#else
#define CHAR_TYPE char
#define RPC_STR RPC_CSTR
#define FORMAT_STORE_CHAR _snprintf_s
#define FORMAT_PRINT printf_s
#endif

int __cdecl wmain(int argc, CHAR_TYPE *ppArgv[])
{
    RPC_STATUS status;
    CHAR_TYPE *pszUuid = NULL;
    CHAR_TYPE *pszProtocolSequence = TEXT("ncalrpc");
    CHAR_TYPE *pszNetworkAddress = NULL;
    CHAR_TYPE szEndpoint[MAX_PATH];
    CHAR_TYPE *pszSpn = NULL;
    CHAR_TYPE *pszOptions = NULL;
    CHAR_TYPE *pszStringBinding = NULL;
    CHAR_TYPE *pszString = TEXT("hello, world");
    RPC_SECURITY_QOS SecQos;
    unsigned long ulCode;

    // allow the user to override settings with command line switches
    if (argc > 1)
    {
        FORMAT_STORE_CHAR(szEndpoint, (unsigned)_countof(szEndpoint), _TRUNCATE, TEXT("%s"), ppArgv[1]);
        //size_t returnValue = 0;
        //mbstowcs_s(&returnValue, szEndpoint, (unsigned)_countof(szEndpoint), ppArgv[1], strlen(ppArgv[1]) + 1);
        //mbstowcs(szEndpoint, ppArgv[1], strlen(ppArgv[1])+1);
    }

    // Use a convenience function to concatenate the elements of
    // the string binding into the proper sequence.
    status = RpcStringBindingCompose(
        (RPC_STR)pszUuid,
        (RPC_STR)pszProtocolSequence,
        NULL,
        (RPC_STR)szEndpoint,
        NULL,
        (RPC_STR*)&pszStringBinding);
    FORMAT_PRINT(TEXT("RpcStringBindingCompose returned 0x%x\n"), status);
    FORMAT_PRINT(TEXT("pszStringBinding = %s\n"), pszStringBinding);
    if (status) {
        exit(status);
    }

    // Set the binding handle that will be used to bind to the server.
    status = RpcBindingFromStringBinding((RPC_STR)pszStringBinding,
        &NCALRPCInterface_v1_0_c_ifspec);
    FORMAT_PRINT(TEXT("RpcBindingFromStringBinding returned 0x%x\n"), status);
    if (status)
    {
        exit(status);
    }

    // Set the quality of service on the binding handle
    SecQos.Version = RPC_C_SECURITY_QOS_VERSION_1;
    SecQos.Capabilities = RPC_C_QOS_CAPABILITIES_DEFAULT;
    SecQos.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
    SecQos.ImpersonationType = RPC_C_IMP_LEVEL_DEFAULT;

    //Set the security provider on binding handle
    status = RpcBindingSetAuthInfoEx(
        NCALRPCInterface_v1_0_c_ifspec,
        NULL,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
        RPC_C_AUTHN_WINNT,
        NULL,
        RPC_C_AUTHN_WINNT,
        &SecQos);

    FORMAT_PRINT(TEXT("RpcBindingSetAuthInfoEx returned 0x%x\n"), status);
    if (status)
    {
        exit(status);
    }

    RpcTryExcept
    {
        uint32_t ret = 0;
        NCALRPC_PACKET_1 pktSend = { 0 };
        pktSend.inSize = 2468;
        NCALRPC_PACKET_2 pktGet = { 0 };
        ret = RPCSend(NCALRPCInterface_v1_0_c_ifspec, &pktSend);
        FORMAT_PRINT(TEXT("Calling the remote procedure 'RPCSend' send %d returns %d\n"), pktSend.inSize, ret);
        ret = RPCGet(NCALRPCInterface_v1_0_c_ifspec, &pktGet);
        FORMAT_PRINT(TEXT("Calling the remote procedure 'RPCGet' get %d returns %d\n"), pktGet.outSize, ret);
    Shutdown(NCALRPCInterface_v1_0_c_ifspec);  // shut down the server side
    }
        RpcExcept((((RpcExceptionCode() != STATUS_ACCESS_VIOLATION) &&
        (RpcExceptionCode() != STATUS_DATATYPE_MISALIGNMENT) &&
            (RpcExceptionCode() != STATUS_PRIVILEGED_INSTRUCTION) &&
            (RpcExceptionCode() != STATUS_BREAKPOINT) &&
            (RpcExceptionCode() != STATUS_STACK_OVERFLOW) &&
            (RpcExceptionCode() != STATUS_IN_PAGE_ERROR) &&
            (RpcExceptionCode() != STATUS_GUARD_PAGE_VIOLATION)
            )
            ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)) {
                ulCode = RpcExceptionCode();
                FORMAT_PRINT(TEXT("Runtime reported exception 0x%lx = %ld\n"), ulCode, ulCode);
            }
    RpcEndExcept
        //  The calls to the remote procedures are complete.
        //  Free the string and the binding handle
        status = RpcStringFree((RPC_STR*)&pszStringBinding);  // remote calls done; unbind
    FORMAT_PRINT(TEXT("RpcStringFree returned 0x%x\n"), status);
    if (status) {
        exit(status);
    }

    status = RpcBindingFree(&NCALRPCInterface_v1_0_c_ifspec);  // remote calls done; unbind
    FORMAT_PRINT(TEXT("RpcBindingFree returned 0x%x\n"), status);
    if (status) {
        exit(status);
    }

    exit(0);
    return 0;
}

/*********************************************************************/
/*                 MIDL allocate and free                            */
/*********************************************************************/

void  __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
    return(malloc(len));
}

void __RPC_USER midl_user_free(void __RPC_FAR * ptr)
{
    free(ptr);
}
