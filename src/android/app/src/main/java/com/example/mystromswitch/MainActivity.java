package com.example.mystromswitch;

// OkHttp : client HTTP
// https://square.github.io/okhttp/
import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;
import okhttp3.MediaType;
import okhttp3.RequestBody;

// JSON (JavaScript Object Notation) est un format d'échange de données textuelles
// https://developer.android.com/reference/org/json/package-summary
import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONException;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;

public class MainActivity extends AppCompatActivity
{
    private final String       TAG                  = "ClientHTTPStrom";
    private OkHttpClient       clientOkHttp         = new OkHttpClient();
    private String             adresseIPSwitchStrom = "";
    private String             url                  = null;
    private Button             boutonDecouvrir;
    private Button             boutonLire;
    private Button             boutonEteindre;
    private Button             boutonAllumer;
    private TextView           titre;
    private TextView           urlRequete;
    private TextView           reponseEtat;
    private TextView           reponseJson;
    private DatagramSocket     socket;
    private final int          PORT_DETECTION_MYSTROM                = 7979;
    public static final int    TIMEOUT_RECEPTION_REPONSE             = 30000;
    public static final String PREFERENCES                           = "preferences";
    public static final String PREFERENCES_ADRESSE_IP_SWITCH_MYSTROM = "adresseIPSwitchStrom";
    SharedPreferences          sharedPreferences;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // On récupère le SharedPreferences à partir du contexte
        sharedPreferences = getBaseContext().getSharedPreferences(PREFERENCES, MODE_PRIVATE);

        // On récupère un élément si il existe
        if(sharedPreferences.contains(PREFERENCES_ADRESSE_IP_SWITCH_MYSTROM))
        {
            // (voir aussi getBoolean(), getFloat(), getInt() ...)
            adresseIPSwitchStrom =
              sharedPreferences.getString(PREFERENCES_ADRESSE_IP_SWITCH_MYSTROM,
                                          ""); // null ou une valeur par défaut
            Toast
              .makeText(getApplicationContext(),
                        "Adresse IP de la prise myStrom : " + adresseIPSwitchStrom,
                        Toast.LENGTH_SHORT)
              .show();
        }

        titre = (TextView)findViewById(R.id.titre);
        if(adresseIPSwitchStrom.isEmpty())
            titre.setText("myStrom Switch");
        else
            titre.setText("myStrom Switch " + adresseIPSwitchStrom);
        urlRequete  = (TextView)findViewById(R.id.urlRequete);
        reponseEtat = (TextView)findViewById(R.id.reponseEtat);
        reponseJson = (TextView)findViewById(R.id.reponseJson);

        boutonDecouvrir = (Button)findViewById(R.id.boutonDecouvrir);
        boutonDecouvrir.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v)
            {
                if(estConnecteReseau())
                {
                    decouvrirAppareilsMyStrom();
                }
                else
                {
                    reponseEtat.setText("Aucune connexion réseau !");
                }
            }
        });
        boutonLire = (Button)findViewById(R.id.boutonLire);
        boutonLire.setEnabled(false);
        boutonLire.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v)
            {
                if(estConnecteReseau())
                {
                    url = "http://" + adresseIPSwitchStrom + "/report";
                    lireEtat(url);
                }
                else
                {
                    reponseEtat.setText("Aucune connexion réseau !");
                }
            }
        });
        boutonEteindre = (Button)findViewById(R.id.boutonEteindre);
        boutonEteindre.setEnabled(false);
        boutonEteindre.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v)
            {
                if(estConnecteReseau())
                {
                    if(!adresseIPSwitchStrom.isEmpty())
                    {
                        url = "http://" + adresseIPSwitchStrom + "/relay?state=0";
                        eteindre(url);
                    }
                }
                else
                {
                    reponseEtat.setText("Aucune connexion réseau !");
                }
            }
        });
        boutonAllumer = (Button)findViewById(R.id.boutonAllumer);
        boutonAllumer.setEnabled(false);
        boutonAllumer.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v)
            {
                if(estConnecteReseau())
                {
                    if(!adresseIPSwitchStrom.isEmpty())
                    {
                        url = "http://" + adresseIPSwitchStrom + "/relay?state=1";
                        allumer(url);
                    }
                }
                else
                {
                    reponseEtat.setText("Aucune connexion réseau !");
                }
            }
        });

        if(!adresseIPSwitchStrom.isEmpty())
        {
            boutonLire.setEnabled(true);
            boutonEteindre.setEnabled(true);
            boutonAllumer.setEnabled(true);
        }
        else
        {
            boutonLire.setEnabled(false);
            boutonEteindre.setEnabled(false);
            boutonAllumer.setEnabled(false);
        }
    }

    private void decouvrirAppareilsMyStrom()
    {
        Log.d(TAG, "decouvrirAppareilsMyStrom()");
        if(socket == null)
        {
            try
            {
                socket = new DatagramSocket(PORT_DETECTION_MYSTROM);
                socket.setSoTimeout(TIMEOUT_RECEPTION_REPONSE);
            }
            catch(SocketException se)
            {
                se.printStackTrace();
            }
        }
        runOnUiThread(new Runnable() {
            @Override
            public void run()
            {
                while(socket != null && !socket.isClosed())
                {
                    byte[] bufferReception = new byte[8];
                    DatagramPacket datagramme =
                      new DatagramPacket(bufferReception, bufferReception.length);

                    try
                    {
                        socket.receive(datagramme);
                        String adresseIPSwitch = datagramme.getAddress().toString();
                        Log.d(TAG,
                              "decouvrirAppareilsMyStrom() emetteurAdresse = " +
                                adresseIPSwitch.substring(1) +
                                " emetteurPort = " + datagramme.getPort());
                        StringBuilder donneesRecues = new StringBuilder();
                        for(byte b: datagramme.getData())
                        {
                            donneesRecues.append(String.format("%02X ", b));
                        }
                        Log.d(TAG,
                              "decouvrirAppareilsMyStrom() donneesRecues = " + donneesRecues +
                                " nbOctets = " + datagramme.getLength());

                        reponseEtat.setText("emetteurAdresse = " + adresseIPSwitch.substring(1));
                        reponseJson.setText("donneesRecues = " + donneesRecues +
                                            " nbOctets = " + datagramme.getLength());

                        if(adresseIPSwitch.substring(1).length() > 0)
                        {
                            adresseIPSwitchStrom = adresseIPSwitch.substring(1);
                            // On sauvegarde un élément (voir aussi putBoolean(), putFloat(),
                            // putInt() ...)
                            sharedPreferences.edit()
                              .putString(PREFERENCES_ADRESSE_IP_SWITCH_MYSTROM,
                                         adresseIPSwitchStrom)
                              .apply(); // ou .commit()
                        }

                        if(!adresseIPSwitchStrom.isEmpty())
                        {
                            boutonLire.setEnabled(true);
                            boutonEteindre.setEnabled(true);
                            boutonAllumer.setEnabled(true);
                        }
                        else
                        {
                            boutonLire.setEnabled(false);
                            boutonEteindre.setEnabled(false);
                            boutonAllumer.setEnabled(false);
                        }
                        return;
                    }
                    catch(IOException e)
                    {
                        throw new RuntimeException(e);
                    }
                }
            }
        });
    }

    private void lireEtat(String url)
    {
        if(clientOkHttp == null)
            return;
        Log.d(TAG, "lireEtat() url = " + url);
        urlRequete.setText(url);
        Request request = new Request.Builder()
                            .url(url)
                            .addHeader("Content-Type", "application/json")
                            .addHeader("Accept", "application/json")
                            .build();

        clientOkHttp.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e)
            {
                e.printStackTrace();
                Log.d(TAG, "lireEtat() onFailure");
                runOnUiThread(new Runnable() {
                    @Override
                    public void run()
                    {
                        reponseEtat.setText("Erreur requête OkHttp !");
                    }
                });
            }

            @Override
            public void onResponse(Call call, Response response) throws IOException
            {
                Log.d(TAG, "lireEtat() onResponse - message = " + response.message());
                Log.d(TAG, "lireEtat() onResponse - code    = " + response.code());

                if(!response.isSuccessful())
                {
                    throw new IOException(response.toString());
                }

                final String body = response.body().string();

                runOnUiThread(new Runnable() {
                    @Override
                    public void run()
                    {
                        if(!response.message().isEmpty())
                            reponseEtat.setText(response.message());
                        else
                            reponseEtat.setText(String.valueOf(response.code()));
                        Log.d(TAG, "lireEtat() body = " + body);
                        reponseJson.setText(body);

                        /*
                            {"power":0,"Ws":0,"relay":true,"temperature":21.5}
                        */

                        JSONObject json = null;

                        try
                        {
                            json = new JSONObject(body);
                            if(json.has("power"))
                            {
                                Log.d(TAG, "lireEtat() power = " + json.getDouble("power"));
                                Log.d(TAG, "lireEtat() Ws    = " + json.getDouble("Ws"));
                                Log.d(TAG, "lireEtat() relay = " + json.getBoolean("relay"));
                                Log.d(TAG,
                                      "lireEtat() temperature = " + json.getDouble("temperature"));
                            }
                        }
                        catch(JSONException e)
                        {
                            e.printStackTrace();
                        }
                    }
                });
            }
        });
    }

    private void eteindre(String url)
    {
        if(clientOkHttp == null)
            return;
        Log.d(TAG, "eteindre() url = " + url);
        urlRequete.setText(url);
        Request request =
          new Request.Builder().url(url).addHeader("Content-Type", "application/json").build();

        clientOkHttp.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e)
            {
                e.printStackTrace();
                Log.d(TAG, "eteindre() onFailure");
                runOnUiThread(new Runnable() {
                    @Override
                    public void run()
                    {
                        reponseEtat.setText("Erreur requête OkHttp !");
                    }
                });
            }

            @Override
            public void onResponse(Call call, Response response) throws IOException
            {
                Log.d(TAG, "eteindre() onResponse - message = " + response.message());
                Log.d(TAG, "eteindre() onResponse - code    = " + response.code());

                if(!response.isSuccessful())
                {
                    throw new IOException(response.toString());
                }

                final String body = response.body().string();

                runOnUiThread(new Runnable() {
                    @Override
                    public void run()
                    {
                        if(!response.message().isEmpty())
                            reponseEtat.setText(response.message());
                        else
                            reponseEtat.setText(String.valueOf(response.code()));
                        reponseJson.setText(body);
                    }
                });
            }
        });
    }

    private void allumer(String url)
    {
        if(clientOkHttp == null)
            return;
        Log.d(TAG, "allumer() url = " + url);
        urlRequete.setText(url);
        Request request =
          new Request.Builder().url(url).addHeader("Content-Type", "application/json").build();

        clientOkHttp.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e)
            {
                e.printStackTrace();
                Log.d(TAG, "allumer() onFailure");
                runOnUiThread(new Runnable() {
                    @Override
                    public void run()
                    {
                        reponseEtat.setText("Erreur requête OkHttp !");
                    }
                });
            }

            @Override
            public void onResponse(Call call, Response response) throws IOException
            {
                Log.d(TAG, "allumer() onResponse - message = " + response.message());
                Log.d(TAG, "allumer() onResponse - code    = " + response.code());

                if(!response.isSuccessful())
                {
                    throw new IOException(response.toString());
                }

                final String body = response.body().string();

                runOnUiThread(new Runnable() {
                    @Override
                    public void run()
                    {
                        if(!response.message().isEmpty())
                            reponseEtat.setText(response.message());
                        else
                            reponseEtat.setText(String.valueOf(response.code()));
                        reponseJson.setText(body);
                    }
                });
            }
        });
    }

    private boolean estConnecteReseau()
    {
        ConnectivityManager connectivityManager =
          (ConnectivityManager)getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo networkInfo = connectivityManager.getActiveNetworkInfo();
        if(networkInfo == null || !networkInfo.isConnected() ||
           (networkInfo.getType() != ConnectivityManager.TYPE_WIFI &&
            networkInfo.getType() != ConnectivityManager.TYPE_MOBILE))
        {
            return false;
        }
        return true;
    }
}