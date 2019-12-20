using System.Collections;
using System.Collections.Generic;
using System;
using System.Text;
using UnityEngine;

public class CELLSendStream
{
    private List<byte> _byteList;

    public CELLSendStream(int nCapacity = 128)
    {
        _byteList = new List<byte>(nCapacity);
    }

    //供其它写入基础数据类型调用
    private void Write(byte[] data)
    {
        _byteList.AddRange(data);
    }

    public void WriteNetCMD(NetCMD netCMD)
    {
        WriteUInt16((UInt16)netCMD);
    }

    /// <summary>
    /// 在字节序列最开始位置插入总的消息长度
    /// </summary>
    public void Finish()
    {
        if(_byteList.Count >= Int16.MaxValue)
        {
            Debug.Log("CELLSendStream len out of max range.");
        }
        UInt16 len = (UInt16)_byteList.Count;
        //2 字节是长度导致的位置增加
        len += 2;
        _byteList.InsertRange(0, BitConverter.GetBytes(len));
    }

    /// <summary>
    /// 转换成 byte[] 序列，方便传输
    /// </summary>
    /// <param name="n"></param>
    public byte[] Array
    {
        get
        {
            return _byteList.ToArray();
        }
    }

    //有符号
    public void WriteInt8(sbyte n)
    {
        _byteList.Add((byte)n);
    }

    public void WriteInt16(Int16 n)
    {
        Write(BitConverter.GetBytes(n));
    }

    public void WriteInt32(Int32 n)
    {
        Write(BitConverter.GetBytes(n));
    }

    public void WriteInt64(Int64 n)
    {
        Write(BitConverter.GetBytes(n));
    }
    //无符号
    public void WriteUInt8(byte n)
    {
        //Write(BitConverter.GetBytes(n));
        _byteList.Add(n);
    }

    public void WriteUInt16(UInt16 n)
    {
        Write(BitConverter.GetBytes(n));
    }

    public void WriteUInt32(UInt32 n)
    {
        Write(BitConverter.GetBytes(n));
    }

    public void WriteUInt64(UInt64 n)
    {
        Write(BitConverter.GetBytes(n));
    }
    //浮点数
    public void WriteFloat(float n)
    {
        Write(BitConverter.GetBytes(n));
    }

    public void WriteDouble(double n)
    {
        Write(BitConverter.GetBytes(n));
    }

    //数组
    /// <summary>
    /// 数组类都需要写入元素个数
    /// </summary>
    /// <param name="data"></param>
    public void WriteBytes(byte[] data)
    {
        //写入元素个数
        WriteUInt32((UInt32)data.Length);
        //写入具体的数据
        Write(data);

    }
    /// <summary>
    /// 字符串必须为 UTF-8 编码格式
    /// 最后多加了一个结束符
    /// </summary>
    /// <param name="str"></param>
    public void WriteString(string str)
    {
        //按照 UTF8 格式处理
        byte[] data = Encoding.UTF8.GetBytes(str);
        //写入元素个数
        WriteUInt32((UInt32)data.Length + 1);
        //写入具体的数据
        Write(data);
        WriteUInt8(0);
    }

    public void WriteInts(Int32[] data)
    {
        //写入元素个数
        WriteUInt32((UInt32)data.Length);
        //写入具体的数据
        for(int n = 0; n < data.Length; n++)
        {
            WriteInt32(data[n]);
        }        
    }
}
