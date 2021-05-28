package JavaSocketClient;

import java.awt.BorderLayout;
import java.awt.event.*;
import java.net.InetAddress;
import java.net.Socket;
import java.io.*;
import javax.swing.*;
 
public class MySocketClient extends JFrame implements Runnable, ActionListener {
   
    JTextArea ta;
    JScrollPane pane;
    JTextField tf;
    Socket s;
    BufferedReader br;
    PrintWriter pw;
   
    public MySocketClient(){
        setTitle("ä�� Ŭ���̾�Ʈ");
        setDefaultCloseOperation(DISPOSE_ON_CLOSE); //����â�� �ݱ�
       
        ta = new JTextArea(); // ui ���� �ڵ�
        ta.setEditable(false);
        pane = new JScrollPane(ta);
        add(pane);
        tf = new JTextField();
        add(tf, BorderLayout.SOUTH);
       
        tf.addActionListener(this); //����ġ�� �̺�Ʈ �߻�
       
        setSize(400,300);
        setVisible(true);
       
       
        //��Ʈ��ũ �ڵ�
        try {
            //s = new Socket(InetAddress.getLocalHost(),5000);
            s = new Socket("192.168.122.179",5000);
            //�Է�
            InputStream is = s.getInputStream();
            InputStreamReader isr = new InputStreamReader(is);
            br = new BufferedReader(isr);
            
            
            //���
            OutputStream os = s.getOutputStream();
            pw = new PrintWriter(os, true);
           
            
           
        } catch (Exception e) {
           
        }
        //������ �ڵ�
        Thread t = new Thread(this);
        t.start();
    }
    @Override
    public void actionPerformed(ActionEvent e) { //���� �Է����� ���
        String chat = tf.getText(); //tf�� ���� ä�� ������ �����´�.
        pw.println(chat);     
        System.out.println("�� : " + chat); // Ȯ�ο� ���
        ta.append("�� : " + chat + "\n");/// ���� �Է��Ѱ� ui �߰�
        
        tf.setText(""); //tf�� �����.       
    }
 
    @Override
    public void run() {
        try {
            while(true){
        		String str= br.readLine(); //ä�� ����
                //str = str.trim();
        		String match = "[^\uAC00-\uD7A3xfe0-9a-zA-Z\\s:/']";
                str =str.replaceAll(match, "");
                //�ѱ������ڵ�(\uAC00-\uD7A3), ���� 0~9(0-9), ���� �ҹ���a~z(a-z), �빮��A~Z(A-Z), ����(\s)
                System.out.println(str); // Ȯ�ο� ���
                ta.append(str+"\n"); // ui�� �߰�              
                
                //��ũ�� �� ������
                ta.setCaretPosition(ta.getText().length());
            }
        } catch (Exception e) {
           
        }
       
 
    }
   
    public static void main(String[] args) {
        new MySocketClient();
    }
 
}