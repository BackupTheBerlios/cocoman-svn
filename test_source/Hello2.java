
import java.io.*;


public class Hello2
{
  static public void main(String[] args)
  {
    try
    {
      BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
      String num = in.readLine();
   
      System.out.println(num + "!!"); 
      System.out.println("Hello world from Java!");

      int i;
      for (i = 0; i < 5; ++i)
        Thread.sleep(1000);
    }
    catch (Exception e) {}

    System.exit(1);
  }
};
