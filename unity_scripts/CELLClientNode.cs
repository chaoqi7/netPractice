using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

public class CELLClientNode : CELLNativeTCPClient
{
    public string IP = "127.0.0.1";
    public short PORT = 4567;
    private bool _bConnect = false;
    // Start is called before the first frame update
    void Start()
    {
        Debug.Log("Start");
        this.Create();
        if (this.Connect(IP, PORT))
        {
            Debug.Log("connect to server success.");
            _bConnect = true;
        }
        else
        {
            Debug.Log("connect to server fail.");
        }

        
    }

    // Update is called once per frame
    void Update()
    {
        Debug.Log("Update");
        if (_bConnect)
        {
            this.OnRun();
        }

        CELLSendStream w = new CELLSendStream();
        w.WriteNetCMD(NetCMD.C2S_STREAM);
        w.WriteInt8(1);
        w.WriteInt16(2);
        w.WriteInt32(3);
        w.WriteInt64(4);
        w.WriteUInt16(5);
        w.WriteFloat(6.7f);
        w.WriteDouble(8.9);

        w.WriteString("client");
        w.WriteString("start 这是中文编码字符 end");
        int[] arr2 = { 10, 11, 12, 13, 15 };
        w.WriteInt32s(arr2);
        w.Finish();

        this.SendData(w.Array);
    }

    void OnDestroy()
    {
        Debug.Log("OnDestroy");
        if (_bConnect)
        {            
            this.Close();
        }
        _bConnect = false;
    }


    public override void OnNetMsg(IntPtr pData, int nLen)
    {
        Debug.Log("CELLClientNode OnNetMsg nLen:" + nLen);
        CELLRecvStream r = new CELLRecvStream(pData, nLen);
        Debug.Log(r.ReadNetLength());
        Debug.Log(r.ReadNetCMD());

        Debug.Log(r.ReadInt8());
        Debug.Log(r.ReadInt16());
        Debug.Log(r.ReadInt32());
        Debug.Log(r.ReadInt64());
        Debug.Log(r.ReadUInt16());

        Debug.Log(r.ReadString());
        Debug.Log(r.ReadString());

        Int32[] arr = r.ReadInt32s();
        for (int n = 0; n < arr.Length; n++)
        {
            Debug.Log("arr" + n + "=" + arr[n]);
        }
        Debug.Log(r.ReadFloat());
        Debug.Log(r.ReadDouble());
    }
}
