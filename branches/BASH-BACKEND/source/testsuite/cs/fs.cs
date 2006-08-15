using System;
using System.IO;

namespace Drew
{
  class DeleteTest
  {
    static void Main(string[] args)
    {
        using (StreamWriter sw = new StreamWriter("somefile")) 
        {
            // Add some text to the file.
            sw.Write("made a new file.");
        }
    }
  }
}
