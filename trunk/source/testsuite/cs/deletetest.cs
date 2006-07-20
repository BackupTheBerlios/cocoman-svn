using System;

namespace Drew
{
  class DeleteTest
  {
    static void Main(string[] args)
    {
      //try {
        System.IO.FileInfo fi = new System.IO.FileInfo("somefile");
        fi.Delete();
        Console.WriteLine("Calling Delete() did not throw");
      //} catch (System.IO.IOException e) {
        //Console.WriteLine(e.Message);
      //}
    }
  }
}
