using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;
using System;
using System.IO;
using System.Net;
using System.Net.Sockets;

namespace InputRedirection
{
    public class Game1 : Game
    {
        GraphicsDeviceManager graphics;
        SpriteBatch spriteBatch;
        Texture2D Font;
        Texture2D Cursor;

        Socket socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, 0);
        IPAddress ipAddress;
        string IPAddress = "192.168.1.2";
        string newIPAddress = "";
        byte[] data = new byte[12];
        uint oldbuttons = 0xFFF;
        uint newbuttons = 0xFFF;
        uint oldtouch = 0x2000000;
        uint newtouch = 0x2000000;
        uint oldcpad = 0x800800;
        uint newcpad = 0x800800;
        uint touchclick = 0x00;
        uint cpadclick = 0x00;
        int Mode = 0;
        Keys[] keysToCheck = { Keys.D0, Keys.D1, Keys.D2, Keys.D3, Keys.D4, Keys.D5, Keys.D6, Keys.D7, Keys.D8, Keys.D9, Keys.NumPad0, Keys.NumPad1, Keys.NumPad2, Keys.NumPad3, Keys.NumPad4, Keys.NumPad5, Keys.NumPad6, Keys.NumPad7, Keys.NumPad8, Keys.NumPad9, Keys.Decimal, Keys.OemPeriod, Keys.Back, Keys.Delete, Keys.Escape };
        Keys[] KeyboardInput = { Keys.A, Keys.S, Keys.N, Keys.M, Keys.H, Keys.F, Keys.T, Keys.G, Keys.W, Keys.Q, Keys.Z, Keys.X, Keys.Right, Keys.Left, Keys.Up, Keys.Down };
        Buttons[] GamePadInput = { Buttons.B, Buttons.A, Buttons.Back, Buttons.Start, Buttons.DPadRight, Buttons.DPadLeft, Buttons.DPadUp, Buttons.DPadDown, Buttons.RightShoulder, Buttons.LeftShoulder, Buttons.Y, Buttons.X };
        Keys[] newKeyboardInput;
        Buttons[] newGamePadInput;
        Keys UpKey;
        bool WaitForKeyUp;
        bool debug = false;
        KeyboardState keyboardState;
        GamePadState gamePadState;
        uint KeyIndex;
        Keys OldKey;

        public Game1()
        {
            graphics = new GraphicsDeviceManager(this);
            Content.RootDirectory = "Content";
        }

        protected override void Initialize()
        {
            this.IsMouseVisible = true;
            this.Window.Title = "InputRedirection";
            graphics.PreferredBackBufferWidth = 320;
            graphics.PreferredBackBufferHeight = 240;
            base.Initialize();
        }

        protected override void LoadContent()
        {
            if (File.Exists("config.cfg"))
            {
                ReadConfig();
            }
            else
            {
                SaveConfig();
            }

            spriteBatch = new SpriteBatch(GraphicsDevice);
            Font = Content.Load<Texture2D>("Fonts\\NESFont");
            Cursor = Content.Load<Texture2D>("Cursors\\Cursor");
        }

        protected override void UnloadContent()
        {

        }

        protected override void Update(GameTime gameTime)
        {
            CheckConnection();
            switch (Mode)
            {
                case 0:
                    {
                        ReadMain();
                    }
                    break;

                case 1:
                    {
                        ReadIPInput();
                    }
                    break;

                case 2:
                    {
                        ReadKeyboardInput();
                    }
                    break;

                case 3:
                    {
                        ReadGamePadInput();
                    }
                    break;

                case 4:
                    {
                        ReadNewKey();
                    }
                    break;

                case 5:
                    {
                        ReadGamePadInput();
                    }
                    break;

            }

            base.Update(gameTime);
        }

        protected override void Draw(GameTime gameTime)
        {
            GraphicsDevice.Clear(Color.Black);
            spriteBatch.Begin();
            {
                switch (Mode)
                {
                    case 0:
                        {
                            ShowMain();
                        }
                        break;

                    case 1:
                        {
                            ShowIPInput();
                        }
                        break;

                    case 2:
                    case 4:
                        {
                            ShowKeyboardInput();
                        }
                        break;

                    case 3:
                    case 5:
                        {
                            ShowGamePadInput();
                        }
                        break;
                }
            }
            spriteBatch.End();

            base.Draw(gameTime);
        }

        private void ReadConfig()
        {
            StreamReader sr = new StreamReader("config.cfg");

            IPAddress = sr.ReadLine();
            if(sr.ReadLine() == "True")
            {
                debug = true;
                this.IsMouseVisible = false;
            }

            for (int i = 0; i < KeyboardInput.Length; i++)
            {
                KeyboardInput[i] = (Keys)Enum.Parse(typeof(Keys), sr.ReadLine());
            }

            /*for (int i = 0; i < GamePadInput.Length; i++)
            {
                GamePadInput[i] = (Buttons)Enum.Parse(typeof(Buttons), sr.ReadLine());
            }*/
            sr.Close();
        }

        private void SaveConfig()
        {
            StreamWriter sw = new StreamWriter("config.cfg");

            sw.WriteLine(IPAddress);
            sw.WriteLine(debug);

            for (int i = 0; i < KeyboardInput.Length; i++)
            {
                sw.WriteLine(KeyboardInput[i]);
            }

            /*for (int i = 0; i < GamePadInput.Length; i++)
            {
                sw.WriteLine(GamePadInput[i]);
            }*/
            sw.Close();
        }

        private void CheckConnection()
        {
            if (!socket.Connected)
            {
                socket.Connect(IPAddress, 4950);
            }

        }

        private void ReadNewKey()
        {
            if (!WaitForKeyUp)
            {
                keyboardState = Keyboard.GetState();

                if(keyboardState.GetPressedKeys().Length > 0)
                {
                    switch (keyboardState.GetPressedKeys()[0])
                    {
                        case Keys.Escape:
                            {
                                KeyboardInput[KeyIndex] = OldKey;
                                Mode = 2;
                            }
                            break;

                        case Keys.F1:
                        case Keys.F2:
                        case Keys.F3:
                        case Keys.F4:
                            {
                                break;
                            }

                        default:
                            {
                                for (int i = 0; i < newKeyboardInput.Length; i++)
                                {
                                    if (keyboardState.GetPressedKeys()[0] == newKeyboardInput[i])
                                    {
                                        break;
                                    }

                                    if (i == (newKeyboardInput.Length - 1))
                                    {
                                        newKeyboardInput[KeyIndex] = keyboardState.GetPressedKeys()[0];
                                        Mode = 2;
                                        WaitForKeyUp = true;
                                        UpKey = keyboardState.GetPressedKeys()[0];
                                    }
                                }
                            }
                            break;
                    }
                }
            }
            else
            {
                if (Keyboard.GetState().IsKeyUp(UpKey))
                {
                    WaitForKeyUp = false;
                }
            }
        }

        private void ReadMain()
        {
            if (!WaitForKeyUp)
            {
                if (Keyboard.GetState().IsKeyDown(Keys.F1))
                {
                    WaitForKeyUp = true;
                    UpKey = Keys.F1;
                    Mode = 1;
                    newIPAddress = IPAddress;
                }

                if (Keyboard.GetState().IsKeyDown(Keys.F2))
                {
                    WaitForKeyUp = true;
                    UpKey = Keys.F2;
                    Mode = 2;
                    newKeyboardInput = KeyboardInput;
                }

                /*if (Keyboard.GetState().IsKeyDown(Keys.F3))
                {
                    WaitForKeyUp = true;
                    UpKey = Keys.F3;
                    Mode = 3;
                    newGamePadInput = GamePadInput;
                }*/

                if (Keyboard.GetState().IsKeyDown(Keys.F4))
                {
                    WaitForKeyUp = true;
                    UpKey = Keys.F4;
                    debug = !debug;
                    this.IsMouseVisible = !this.IsMouseVisible;
                    SaveConfig();
                }
            }
            else
            {
                if (Keyboard.GetState().IsKeyUp(UpKey))
                {
                    WaitForKeyUp = false;
                }
            }

            keyboardState = Keyboard.GetState();
            gamePadState = GamePad.GetState(PlayerIndex.One);
            newbuttons = 0x00;
            for (int i = 0; i < GamePadInput.Length; i++)
            {
                switch (GamePadInput[i])
                {
                    case Buttons.DPadUp:
                        {
                            if ((GamePad.GetState(PlayerIndex.One).DPad.Up == ButtonState.Pressed) || keyboardState.IsKeyDown(KeyboardInput[i]))
                            {
                                newbuttons += (uint)(0x01 << i);
                            }
                        }
                        break;

                    case Buttons.DPadDown:
                        {
                            if ((GamePad.GetState(PlayerIndex.One).DPad.Down == ButtonState.Pressed) || keyboardState.IsKeyDown(KeyboardInput[i]))
                            {
                                newbuttons += (uint)(0x01 << i);
                            }
                        }
                        break;

                    case Buttons.DPadLeft:
                        {
                            if ((GamePad.GetState(PlayerIndex.One).DPad.Left == ButtonState.Pressed) || keyboardState.IsKeyDown(KeyboardInput[i]))
                            {
                                newbuttons += (uint)(0x01 << i);
                            }
                        }
                        break;

                    case Buttons.DPadRight:
                        {
                            if ((GamePad.GetState(PlayerIndex.One).DPad.Right == ButtonState.Pressed) || keyboardState.IsKeyDown(KeyboardInput[i]))
                            {
                                newbuttons += (uint)(0x01 << i);
                            }
                        }
                        break;

                    default:
                        {
                            if (gamePadState.IsButtonDown(GamePadInput[i]) || keyboardState.IsKeyDown(KeyboardInput[i]))
                            {
                                newbuttons += (uint)(0x01 << i);
                            }
                        }
                        break;
                }
            }

            newbuttons ^= 0xFFF;

            //Touch
            if (Mouse.GetState().LeftButton == ButtonState.Pressed)
            {
                TouchInput(ref newtouch, ref touchclick, false);
            }
            else
            {
                touchclick = 0x00;
                if (GamePad.GetState(PlayerIndex.One).Buttons.RightStick == ButtonState.Pressed)
                {
                    newtouch = (uint)Math.Round(2047.5 + (GamePad.GetState(PlayerIndex.One).ThumbSticks.Right.X * 2047.5));
                    newtouch += (uint)Math.Round(2047.5 + (GamePad.GetState(PlayerIndex.One).ThumbSticks.Right.Y * 2047.5)) << 0x0C;
                    newtouch += 0x1000000;
                }
                else
                {
                    newtouch = 0x2000000;
                }
            }

            //Circle Pad
            if (Mouse.GetState().RightButton == ButtonState.Pressed)
            {
                TouchInput(ref newcpad, ref cpadclick, true);
            }
            else
            {
                cpadclick = 0x00;
                newcpad = (uint)Math.Round(2047.5 + (GamePad.GetState(PlayerIndex.One).ThumbSticks.Left.X * 2047.5));
                newcpad += (uint)Math.Round(4095 - (2047.5 + (GamePad.GetState(PlayerIndex.One).ThumbSticks.Left.Y * 2047.5))) << 0x0C;

                if (newcpad == 0x800800)
                {

                    if (Keyboard.GetState().IsKeyDown(KeyboardInput[12]))
                    {
                        newcpad = 0xFFF + (((newcpad >> 0x0C) & 0xFFF) << 0x0C);
                    }

                    if (Keyboard.GetState().IsKeyDown(KeyboardInput[13]))
                    {
                        newcpad = (((newcpad >> 0x0C) & 0xFFF) << 0x0C);
                    }

                    if (Keyboard.GetState().IsKeyDown(KeyboardInput[15]))
                    {
                        newcpad = (newcpad & 0xFFF) + (0x00 << 0x0C);
                    }

                    if (Keyboard.GetState().IsKeyDown(KeyboardInput[14]))
                    {
                        newcpad = (newcpad & 0xFFF) + (0xFFF << 0x0C);
                    }
                }

                if (newcpad != 0x800800)
                {
                    newcpad += 0x1000000;
                }
            }

            SendInput();
        }

        private void ReadIPInput()
        {
            if (!WaitForKeyUp)
            {
                for (int i = 0; i < keysToCheck.Length; i++)
                {
                    if (Keyboard.GetState().IsKeyDown(keysToCheck[i]))
                    {
                        WaitForKeyUp = true;
                        UpKey = keysToCheck[i];
                        switch (keysToCheck[i])
                        {
                            case Keys.Back:
                            case Keys.Delete:
                                {
                                    if (newIPAddress.Length != 0)
                                    {
                                        newIPAddress = newIPAddress.Substring(0, newIPAddress.Length - 1);
                                    }
                                }
                                break;

                            case Keys.Escape:
                                {
                                    if (System.Net.IPAddress.TryParse(newIPAddress, out ipAddress))
                                    {
                                        Mode = 0;
                                        IPAddress = ipAddress.ToString();
                                        SaveConfig();
                                    }
                                }
                                break;

                            default:
                                {
                                    if (newIPAddress.Length < 15)
                                    {
                                        newIPAddress += KeytoText(keysToCheck[i]);
                                    }
                                }
                                break;
                        }
                    }
                }
            }
            else
            {
                if (Keyboard.GetState().IsKeyUp(UpKey))
                {
                    WaitForKeyUp = false;
                }
            }
        }

        private void ReadKeyboardInput()
        {
            if (!WaitForKeyUp)
            {
                for (int i = 0; i < newKeyboardInput.Length; i++)
                {
                    if (Keyboard.GetState().IsKeyDown(newKeyboardInput[i]))
                    {
                        WaitForKeyUp = true;
                        UpKey = newKeyboardInput[i];
                        OldKey = newKeyboardInput[i];
                        newKeyboardInput[i] = Keys.None;
                        KeyIndex = (uint)i;
                        Mode = 4;
                    }
                }

                if (Keyboard.GetState().IsKeyDown(Keys.Escape))
                {
                    Mode = 0;
                    KeyboardInput = newKeyboardInput;
                    WaitForKeyUp = true;
                    UpKey = Keys.Escape;
                    SaveConfig();
                }
            }
            else
            {
                if (Keyboard.GetState().IsKeyUp(UpKey))
                {
                    WaitForKeyUp = false;
                }
            }
        }

        private void ReadGamePadInput()
        {

        }

        private void ShowMain()
        {
            if (debug)
            {
                DrawString(8, 8, "IPAddress : " + IPAddress, Color.White);
                DrawString(8, 16, "  Buttons : " + oldbuttons.ToString("X8"), Color.White);
                DrawString(8, 24, "    Touch : " + oldtouch.ToString("X8"), Color.White);
                DrawString(8, 32, "     CPad : " + oldcpad.ToString("X8"), Color.White);

                int mousex = Mouse.GetState().Position.X;
                int mousey = Mouse.GetState().Position.Y;
                if (oldtouch == 0x2000000)
                {
                    if ((GamePad.GetState(PlayerIndex.One).ThumbSticks.Right.X == 0.0) && (GamePad.GetState(PlayerIndex.One).ThumbSticks.Right.Y == 0.0))
                    {
                        if (MouseInWindow(mousex, mousey))
                        {
                            spriteBatch.Draw(Cursor, new Rectangle(mousex - 1, mousey - 1, 3, 3), Color.Red);
                        }
                    }
                    else
                    {
                        int stickx = (int)Math.Round(159.5 + (GamePad.GetState(PlayerIndex.One).ThumbSticks.Right.X * 159.5));
                        int sticky = (int)Math.Round(119.5 + (GamePad.GetState(PlayerIndex.One).ThumbSticks.Right.Y * 119.5));
                        spriteBatch.Draw(Cursor, new Rectangle(stickx - 1, sticky - 1, 3, 3), Color.Red);
                    }
                }
                else
                {
                    int touchx = (int)Math.Round(((double)(oldtouch & 0xFFF) / 0xFFF) * 319);
                    int touchy = (int)Math.Round(((double)((oldtouch >> 0x0C) & 0xFFF) / 0xFFF) * 239);
                    spriteBatch.Draw(Cursor, new Rectangle(touchx - 1, touchy - 1, 3, 3), Color.Green);
                }

                int cpadx = (int)Math.Round(((double)(oldcpad & 0xFFF) / 0xFFF) * 319);
                int cpady = (int)Math.Round(239 - (((double)((oldcpad >> 0x0C) & 0xFFF) / 0xFFF) * 239));
                spriteBatch.Draw(Cursor, new Rectangle(cpadx - 1, cpady - 1, 3, 3), Color.Blue);
            }
        }

        private void ShowIPInput()
        {
            DrawString(0, 0, "IP Address: " + newIPAddress, Color.White);
        }

        private void ShowKeyboardInput()
        {
            DrawString(68, 36, "DPad Up    : " + newKeyboardInput[6], Color.White);
            DrawString(68, 44, "DPad Down  : " + newKeyboardInput[7], Color.White);
            DrawString(68, 52, "DPad Left  : " + newKeyboardInput[5], Color.White);
            DrawString(68, 60, "DPad Right : " + newKeyboardInput[4], Color.White);

            DrawString(68, 76, "CPad Up    : " + newKeyboardInput[14], Color.White);
            DrawString(68, 84, "CPad Down  : " + newKeyboardInput[15], Color.White);
            DrawString(68, 92, "CPad Left  : " + newKeyboardInput[13], Color.White);
            DrawString(68, 100, "CPad Right : " + newKeyboardInput[12], Color.White);
        

            DrawString(68, 116, "A          : " + newKeyboardInput[0], Color.White);
            DrawString(68, 124, "B          : " + newKeyboardInput[1], Color.White);
            DrawString(68, 132, "Y          : " + newKeyboardInput[11], Color.White);
            DrawString(68, 140, "X          : " + newKeyboardInput[10], Color.White);

            DrawString(68, 156, "L          : " + newKeyboardInput[9], Color.White);
            DrawString(68, 164, "R          : " + newKeyboardInput[8], Color.White);
            DrawString(68, 172, "Start      : " + newKeyboardInput[3], Color.White);
            DrawString(68, 180, "Select     : " + newKeyboardInput[2], Color.White);
        }

        private void ShowGamePadInput()
        {
            DrawString(68, 36, "DPad Up    : " + newGamePadInput[6], Color.White);
            DrawString(68, 44, "DPad Down  : " + newGamePadInput[7], Color.White);
            DrawString(68, 52, "DPad Left  : " + newGamePadInput[5], Color.White);
            DrawString(68, 60, "DPad Right : " + newGamePadInput[4], Color.White);

            DrawString(68, 76, "CPad Up    : LeftStickUp", Color.Gray);
            DrawString(68, 84, "CPad Down  : LeftStickDown", Color.Gray);
            DrawString(68, 92, "CPad Left  : LeftStickLeft", Color.Gray);
            DrawString(68, 100, "CPad Right : LeftStickRight", Color.Gray);


            DrawString(68, 116, "A          : " + newGamePadInput[0], Color.White);
            DrawString(68, 124, "B          : " + newGamePadInput[1], Color.White);
            DrawString(68, 132, "Y          : " + newGamePadInput[11], Color.White);
            DrawString(68, 140, "X          : " + newGamePadInput[10], Color.White);

            DrawString(68, 156, "L          : " + newGamePadInput[9], Color.White);
            DrawString(68, 164, "R          : " + newGamePadInput[8], Color.White);
            DrawString(68, 172, "Start      : " + newGamePadInput[3], Color.White);
            DrawString(68, 180, "Select     : " + newGamePadInput[2], Color.White);
        }

        private string KeytoText(Keys key)
        {
            string result = "";

            switch (key)
            {
                case Keys.NumPad0:
                case Keys.NumPad1:
                case Keys.NumPad2:
                case Keys.NumPad3:
                case Keys.NumPad4:
                case Keys.NumPad5:
                case Keys.NumPad6:
                case Keys.NumPad7:
                case Keys.NumPad8:
                case Keys.NumPad9:
                    {
                        result = key.ToString().Substring(6);
                    }
                    break;

                case Keys.D0:
                case Keys.D1:
                case Keys.D2:
                case Keys.D3:
                case Keys.D4:
                case Keys.D5:
                case Keys.D6:
                case Keys.D7:
                case Keys.D8:
                case Keys.D9:
                    {
                        result = key.ToString().Substring(1);
                    }
                    break;

                case Keys.Decimal:
                case Keys.OemPeriod:
                    {
                        result = ".";
                    }
                    break;
            }
            return result;
        }

        private void DrawString(int X, int Y, string data, Color color)
        {
            int TexX = 0;
            int TexY = 0;
            for (int i = 0; i < data.Length; i++)
            {
                TexX = ((data[i]) & 0x0F) * 0x08;
                TexY = (((data[i]) & 0xF0) >> 0x04) * 0x08;
                spriteBatch.Draw(Font, new Rectangle(X, Y, 8, 8), new Rectangle(TexX, TexY, 8, 8), color);
                X += 8;
            }
        }

        private bool MouseInWindow(int x, int y)
        {
            bool result = false;
            if (((x >= 0) && (x < 320)) && ((y >= 0) && (y < 240)))
            {
                result = true;
            }
            return result;
        }

        private void ClampValues(ref int x, ref int y)
        {
            if (x < 0)
            {
                x = 0;
            }

            if (y < 0)
            {
                y = 0;
            }

            if (x > 319)
            {
                x = 319;
            }

            if (y > 239)
            {
                y = 239;
            }
        }

        private void TouchInput(ref uint value, ref uint mouseclick, bool cpad)
        {
            int X = Mouse.GetState().Position.X;
            int Y = Mouse.GetState().Position.Y;

            if (mouseclick == 0x00)
            {
                if (MouseInWindow(X, Y))
                {
                    mouseclick = 0x01;
                }
                else
                {
                    mouseclick = 0x02;
                }
            }

            if (mouseclick == 0x01)
            {
                ClampValues(ref X, ref Y);
                X = (int)Math.Round(((double)X / 319) * 4095);
                if (cpad)
                {
                    Y = (int)(4095 - Math.Round(((double)Y / 239) * 4095));
                }
                else
                {
                    Y = (int)Math.Round(((double)Y / 239) * 4095);
                }
                value = (uint)X + ((uint)Y << 0x0C) + 0x01000000;
            }
        }

        private void SendInput()
        {
            if (socket.Connected)
            {
                if ((newbuttons != oldbuttons) || (newtouch != oldtouch) || (newcpad != oldcpad))
                {
                    oldbuttons = newbuttons;
                    oldtouch = newtouch;
                    oldcpad = newcpad;

                    //Buttons
                    data[0x00] = (byte)(oldbuttons & 0xFF);
                    data[0x01] = (byte)((oldbuttons >> 0x08) & 0xFF);
                    data[0x02] = (byte)((oldbuttons >> 0x10) & 0xFF);
                    data[0x03] = (byte)((oldbuttons >> 0x18) & 0xFF);

                    //Touch
                    data[0x04] = (byte)(oldtouch & 0xFF);
                    data[0x05] = (byte)((oldtouch >> 0x08) & 0xFF);
                    data[0x06] = (byte)((oldtouch >> 0x10) & 0xFF);
                    data[0x07] = (byte)((oldtouch >> 0x18) & 0xFF);

                    //CPad
                    data[0x08] = (byte)(oldcpad & 0xFF);
                    data[0x09] = (byte)((oldcpad >> 0x08) & 0xFF);
                    data[0x0A] = (byte)((oldcpad >> 0x10) & 0xFF);
                    data[0x0B] = (byte)((oldcpad >> 0x18) & 0xFF);

                    try
                    {
                        socket.Send(data);
                    }
                    catch { }
                }
            }
        }
    }
}
