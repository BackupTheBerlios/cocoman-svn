
import java.io.*;


public class Hello
{
  static public void main(String[] args)
  {
    System.out.println("Hello world from Java!");
    try
    {
      for (;;)
        Thread.sleep(1000);
    }
    catch (Exception e) {}
  }
};
