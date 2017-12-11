package cse379.driveralert;

import android.util.Log;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketTimeoutException;

public class TcpClient {

    public static final String SERVER_IP = "172.31.5.253";               // todo: change the IP address for a Raspberry Pi
    public static final int SERVER_PORT = 8080;                          // todo: change to the port number the server connects to
    private String mServerMessage;                                       // message to send to the server
    private OnMessageReceived mMessageListener = null;                   // sends message received notifications
    private boolean mRun = false;                                        // while this is true, the server will continue running
    private BufferedReader mBufferIn;                                    // used to read messages from the server
    Socket socket;                                                       // TCP socket which connects client to server

    /**
     * TcpClient: Constructor of the class.
     * @param listener listens for the message received from the server
     */
    public TcpClient(OnMessageReceived listener) {
        mMessageListener = listener;
    }

    /**
     * connect: connects the client to the server with the IP and port number. If no connection
     * is created after 5000 milliseconds, the method tries again.
     * @return returns the socket that connects the client to the server
     * @throws IOException whenever there is a keyboard interrupt
     */
    public Socket connect() throws IOException {
        // Establish connection, create the socket
        Socket socket_temp;
        while (true) {
            try {
                socket_temp = new Socket();
                socket_temp.connect(new InetSocketAddress(SERVER_IP, SERVER_PORT), 5000);
                System.out.println("Connection established!");
                break;
            } catch(SocketTimeoutException e) {
                System.out.println("SocketTimeout, retrying...");
                continue;
            }
        }
        return socket_temp;
    }

    /**
     * listen: gets the classification from the classifier. Notifies the ConnectTask object to
     * change the view and sound the notification.
     * @return returns 0 if the driver is not distracted, 1 if distracted, and -1 if error
     * @param socket : the active socket to be used for getting the classifier's results
     */
    private int listen(Socket socket) {
        try {
            //receives the message which the server sends back
            if (mBufferIn == null)
                mBufferIn = new BufferedReader(new InputStreamReader(socket.getInputStream()));

            //in this while the client listens for the messages sent by the server
            mServerMessage = mBufferIn.readLine();

            System.out.println((mServerMessage == null || mServerMessage.isEmpty()) ? "[EMPTY]" : mServerMessage);

            if (mServerMessage != null && mMessageListener != null) {
                //call the method messageReceived from MyActivity class
                mMessageListener.messageReceived(mServerMessage);
            }

            if("1".equals(mServerMessage)) {
                return 1;

            } else if("0".equals(mServerMessage)){
                return 0;
            } else {
                return -1;
            }
        }
        catch (Exception e) {
            Log.e("TCP", "S: Error", e);
            return -1;
        }
    }

    /**
     * closeConnection: closes the socket connection
     * @param socket the active socket
     * @return true if the socket has been properly closed and false otherwise.
     */
    public boolean closeConnection(Socket socket) {
        try {
            socket.close();
            mBufferIn = null;
            return true;
        } catch (Exception e) {
            Log.e("Socket", "Closing: Error", e);
            return false;
        }
    }

    /**
     * run: calls the connect method, and checks if the socket receives a signal. checks if socket is closed.
     * @throws IOException handles a keyboard interrupt
     */
    public void run() throws IOException {
        mRun = true;

        try {
            socket = connect();
        } catch (Exception e) {
            Log.e("Socket", "Connect: Error", e);
        }

        mMessageListener.messageReceived("0");
        for (int response = listen(socket); response != -1; response = listen(socket)) {
            if (response == 1) {
                System.out.println("distracted");
            }
            else if (response ==0) {
                System.out.println("not distracted");
            }
        }
        if (closeConnection(socket)) {
            mMessageListener.messageReceived("-1");
        }
    }

    /**
     * OnMessageReceived: declare the interface. The method messageReceived(String message) will must be
     * implemented in the MyActivity class at on asyncTask doInBackground
     */
    public interface OnMessageReceived {
        //Declare the interface.
        public void messageReceived(String message);
    }
}