<?php
if (array_key_exists("id", $_GET) && array_key_exists($_GET["id"], $people)) {
?>  
  
  <!--Submission form-->
  <div id="box">
    <form method="post" enctype="multipart/form-data" action="submit-file.php">
    <h3>Submit a solution</h3>


<?php
    $user_id = $_GET["id"];
    printf("User ID: <input type=\"text\" name=\"id\" value=\"%s\" /><br />", $user_id);
?>

    <label>Language:</label>
    <select name="lang">
      <option value="auto">Auto</option>
      <option value="c">C</option>
      <option value="cpp">C++</option>
      <option value="java">Java</option>
    </select><br />
    <label>Problem Number:</label>
    <select name="progno">
    <?php
        for ($i = 1; $i <= $NUM_PROBLEMS; $i++) {
            printf("<option value=\"Problem%s\">Problem %s</option>\n", $i, $i);
        }
    ?>
    </select><br />
    <label>Source file:</label><input type="file" name="program" onfocus="clickedBrowse()" onclick="clickedBrowse()" /><br />
    <div id="refreshStatus"></div>
    <input type="hidden" name="MAX_FILE_SIZE" value="102400" />
    <input type="submit" value="Submit" />
    </form>

  </div>

<?php
}
?>
