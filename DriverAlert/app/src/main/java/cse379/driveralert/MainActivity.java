package cse379.driveralert;

import android.media.MediaPlayer;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;

import java.io.IOException;

public class MainActivity extends AppCompatActivity {
    TcpClient mTcpClient;

    /**
     * onCreate: starts as soon as the app opens. creates the savedInstanceState. Initializes ConnectTask object.
     * @param savedInstanceState
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        System.out.println("1");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.no_connection);

        new ConnectTask().execute("");
    }

    /**
     * ConnectTask: an AsyncTask class that will run in the background and change UI threads as we receive
     * signals from the server side.
     */
    public class ConnectTask extends AsyncTask<String, String, TcpClient> {

        MediaPlayer mp = MediaPlayer.create(MainActivity.this, R.raw.ppa);
        // sound credits to https://notificationsounds.com/tags/android?page=5. Sound name = stealthy beeps

        /**
         * doInBackground:
         * @param message declares and initializes a TcpClient object to connect a socket to the server
         * @return returns a TcpClient object.
         */
        @Override
        public TcpClient doInBackground(String... message) {
            // we crate a TCPClient object
            mTcpClient = new TcpClient(new TcpClient.OnMessageReceived() {
                @Override
                // here the messageReceived method is implemented
                public void messageReceived(String message) {
                    publishProgress(message);
                }
            });
            try {
                mTcpClient.run(); // establishes connection.
            } catch (IOException e) {
                e.printStackTrace();
            }
            return null;
        }

        /**
         * onProgresUpdate: updates the UI thread. Changes the view and sounds the notification.
         * @param values
         */
        @Override
        protected void onProgressUpdate(String... values) {
            super.onProgressUpdate(values);
            if (values[0].equals("1")) {        // distracted
                setContentView(R.layout.alert);
                mp.start();
            }
            else if(values[0].equals("0")) {    // not distracted
                setContentView(R.layout.no_alert);
            }
            else {
                setContentView(R.layout.no_connection);     // returned -1. Server socket closed
            }
        }
    }
}
