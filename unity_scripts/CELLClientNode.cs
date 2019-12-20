using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CELLClientNode : CELLNativeTCPClient
{
    public string IP = "127.0.0.1";
    public short PORT = 4567;
    private bool _bConnect = false;
    // Start is called before the first frame update
    void Start()
    {
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
        w.WriteString("haha");
        int[] arr2 = { 10, 11, 12, 13, 15 };
        w.WriteInts(arr2);
        w.Finish();

        this.SendData(w.Array);
    }

    // Update is called once per frame
    void Update()
    {
        if (_bConnect)
        {
            this.OnRun();
        }
        
        //SendData()
    }

    //     void OnDestroy()
    //     {
    //         if (_bConnect)
    //         {
    //             Debug.Log("OnDestroy");
    //             this.Close();
    //         }
    // 
    //     }


    public override void OnNetMsg(byte[] pData)
    {
        Debug.Log("reTransform to other." + pData.Length);
    }
}
