using System;

namespace ResidentWin.Models
{
    /// <summary>
    /// BLE接続状態を表す列挙型
    /// </summary>
    public enum ConnectionState
    {
        /// <summary>
        /// 切断中
        /// </summary>
        Disconnected,

        /// <summary>
        /// 接続待機中 (Advertising)
        /// </summary>
        Waiting,

        /// <summary>
        /// 接続中
        /// </summary>
        Connected,

        /// <summary>
        /// ペアリング中
        /// </summary>
        Pairing,

        /// <summary>
        /// エラー
        /// </summary>
        Error
    }

    /// <summary>
    /// 接続状態の変化を通知するイベント引数
    /// </summary>
    public class ConnectionStateChangedEventArgs : EventArgs
    {
        public ConnectionState State { get; set; }
        public string? Message { get; set; }
        public DateTime Timestamp { get; set; }

        public ConnectionStateChangedEventArgs(ConnectionState state, string? message = null)
        {
            State = state;
            Message = message;
            Timestamp = DateTime.Now;
        }
    }
}
