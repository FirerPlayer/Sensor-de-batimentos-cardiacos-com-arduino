<html>
  <head>
    <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
    <script type="text/javascript">
      google.charts.load('current', {'packages':['corechart']});
      google.charts.setOnLoadCallback(drawChart);

      function drawChart() {
        var data = google.visualization.arrayToDataTable([
          ['Tempo', 'Batimentos (bpm)','Oxigenação (%)', {role: 'annotation'}],
          <?php
          include 'conection.php';
          $sql = "SELECT * FROM projbc";
          $buscar = mysqli_query($conection,$sql);

          while ($dados = mysqli_fetch_array($buscar)) {
            $tempo = $dados['tempo'];
            $batimentos = $dados['batimentos'];
            $oxi = $dados['oxi'];
            
          ?>
          ['<?php echo $tempo ?>', <?php echo $batimentos ?>, <?php echo $oxi ?>, <?php echo $oxi ?>],
          <?php }?>
        ]);

        var options = {
          title: 'Batimentos Cardíaco e Oxigenação do sangue',
          curveType: 'function',
          legend: { position: 'bottom' }
        };

        var chart = new google.visualization.LineChart(document.getElementById('curve_chart'));

        chart.draw(data, options);
      }
    </script>
  </head>
  <body>
    <div id="curve_chart" style="width: 900px; height: 500px"></div>
  </body>
</html>
