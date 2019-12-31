
using UnityEngine;
using System.Runtime.InteropServices;
using AOT;
using System;

public enum NetCMD
{
    C2S_LOGIN,
    S2C_LOGIN,
    C2S_LOGOUT,
    S2C_LOGOUT,
    S2C_NEW_USER_JOIN,
    C2S_HEART,
    S2C_HEART,
    C2S_STREAM,
    S2C_STREAM,
    S2C_ERROR,
};

public class CELLNativeTCPClient : MonoBehaviour
{
    //回调函数代理，从 DLL 里面调用此函数
    public delegate void OnNetMsgCallback(IntPtr csObj, IntPtr pData, int nLen);
    //回调函数代理，从 DLL 里面调用此函数
    [MonoPInvokeCallback(typeof(OnNetMsgCallback))]
    public void OnNetMsg(IntPtr csObj, IntPtr pData, int nLen)
    {
        Debug.Log("Callback OnNetMsg length:" + nLen);
        GCHandle csHandle = GCHandle.FromIntPtr(csObj);
        CELLNativeTCPClient csThisObj = csHandle.Target as CELLNativeTCPClient;
        if (csThisObj)
        {
            csThisObj.OnNetMsg(pData, nLen);
        }
    }


#if UNITY_IPHONE
    [DllImport ("__Internal")]
#else
    [DllImport("CPPNet")]
#endif
    private static extern IntPtr CELLNativeTCPClient_Create(IntPtr csObj, OnNetMsgCallback cb);

#if UNITY_IPHONE
    [DllImport ("__Internal")]
#else
    [DllImport("CPPNet")]
#endif
    private static extern bool CELLNativeTCPClient_Connect(IntPtr cppObj, string ip, short port);

#if UNITY_IPHONE
    [DllImport ("__Internal")]
#else
    [DllImport("CPPNet")]
#endif
    private static extern bool CELLNativeTCPClient_OnRun(IntPtr cppObj);

#if UNITY_IPHONE
    [DllImport ("__Internal")]
#else
    [DllImport("CPPNet")]
#endif
    private static extern void CELLNativeTCPClient_Close(IntPtr cppObj);

#if UNITY_IPHONE
    [DllImport ("__Internal")]
#else
    [DllImport("CPPNet")]
#endif
    private static extern int CELLNativeTCPClient_SendData(IntPtr cppObj, byte[] pData, int nLen);



    //this 对象的 handle
    GCHandle _handleThis;
    //this 对象的指针，传递到 DLL 面使用
    IntPtr _handleThisIntPtr = IntPtr.Zero;
    //DLL 里面生成的对象指针，返回在 C# 里面使用
    IntPtr _cppObjectPtr = IntPtr.Zero;
    public void Create()
    {
        _handleThis = GCHandle.Alloc(this);
        _handleThisIntPtr = GCHandle.ToIntPtr(_handleThis);
        _cppObjectPtr = CELLNativeTCPClient_Create(_handleThisIntPtr, OnNetMsg);
    }

    public bool Connect(string ip, short port)
    {
        if (_cppObjectPtr == IntPtr.Zero)
            return false;
        return CELLNativeTCPClient_Connect(_cppObjectPtr, ip, port);
    }

    public bool OnRun()
    {
        if (_cppObjectPtr == IntPtr.Zero)
            return false;
        return CELLNativeTCPClient_OnRun(_cppObjectPtr);
    }

    public void Close()
    {
        //释放 dll 返回的对象
        if (_cppObjectPtr != IntPtr.Zero)
        {
            CELLNativeTCPClient_Close(_cppObjectPtr);
            _cppObjectPtr = IntPtr.Zero;
        }
        //释放this
        if (_handleThisIntPtr != IntPtr.Zero)
        {
            _handleThis.Free();
            _handleThisIntPtr = IntPtr.Zero;
        }
    }

    public int SendData(byte[] pData)
    {
        if (_cppObjectPtr == IntPtr.Zero)
            return 0;
        return CELLNativeTCPClient_SendData(_cppObjectPtr, pData, pData.Length);
    }

    public virtual void OnNetMsg(IntPtr pData, int nLen)
    {
        Debug.Log("CELLNativeTCPClient OnNetMsg nLen=" + nLen);
    }
}
