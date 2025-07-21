pipeline {
  agent { label 'windows-agent' }

  parameters {
    string(name: 'RUN_ID', defaultValue: '', description: 'GitHub Actions run ID')
    string(name: 'BATCH_ID', defaultValue: '', description: 'Batch ID for each run')
  }

  environment {
    GITHUB_OWNER = 'sihjiaqi'
    GITHUB_REPO  = 'ameba-rtos-pro2'
    GITHUB_TOKEN = credentials('GITHUB_TOKEN')  // Store GitHub PAT in Jenkins Credentials
    BOARD = 'Ameba_AMB82-MINI'
  }

  stages {
    stage('Test GitHub Token') {
      steps {
        script {
          if (GITHUB_TOKEN) {
            echo "Length of token: ${GITHUB_TOKEN.length()}"
          } else {
            echo "GITHUB_TOKEN is not defined!"
          }
        }
      }
    }

    stage('Setup Tools Folder') {
      steps {
        script {
          def toolsFolder = ""
          def imageExe = ""
          def toolsSource = ""

          if (isUnix()) {
            def unameOut = sh(script: 'uname', returnStdout: true).trim()
            if (unameOut == "Darwin") {
              echo "Detected macOS"
              toolsFolder = "${env.WORKSPACE}/unzipped_artifacts/ameba_pro2_tools_macos"
              toolsSource = "ameba-arduino-pro2-dev/Arduino_package/ameba_pro2_tools_macos"
              imageExe    = "${toolsFolder}/image_macos"
            } else {
              echo "Detected Linux"
              toolsFolder = "${env.WORKSPACE}/unzipped_artifacts/ameba_pro2_tools_linux"
              toolsSource = "ameba-arduino-pro2-dev/Arduino_package/ameba_pro2_tools_linux"
              imageExe    = "${toolsFolder}/image_linux"
            }
          } else {
            echo "Detected Windows"
            toolsFolder = "${env.WORKSPACE}\\unzipped_artifacts\\ameba_pro2_tools_windows"
            toolsSource = "ameba-arduino-pro2-dev\\Arduino_package\\ameba_pro2_tools_windows"
            imageExe    = "${toolsFolder}\\image_windows.exe"
          }
          env.TOOLS_SOURCE = toolsSource
          env.TOOLS_FOLDER = toolsFolder
          env.IMAGE_EXE = imageExe
        }
      }
    }

    stage('Download Firmware Artifacts') {
      steps {
        script {
          echo "Run ID: ${params.RUN_ID} | Batch ID: ${params.BATCH_ID}"

          // Unix version
          if (isUnix()) {  
            sh """
            curl -s -H "Authorization: token ${GITHUB_TOKEN}" \\
              https://api.github.com/repos/${GITHUB_OWNER}/${GITHUB_REPO}/actions/runs/${params.RUN_ID}/artifacts \\
              > artifacts.json
            """

            def artifactsJson = readFile('artifacts.json').trim()
            def parsed = readJSON text: artifactsJson
            def artifacts = parsed.artifacts

            for (artifact in artifacts) {
              if (artifact.name.contains(params.BATCH_ID)) {
                echo "Downloading artifact: ${artifact.name} (ID: ${artifact.id})"
                env.ARTIFACT_NAME = artifact.name

                try {
                  sh """
                  mkdir -p artifact_files/${artifact.name}
                  curl -L -H "Authorization: token ${GITHUB_TOKEN}" \\
                    -o ${artifact.name}.zip \\
                    https://api.github.com/repos/${GITHUB_OWNER}/${GITHUB_REPO}/actions/artifacts/${artifact.id}/zip
                  unzip -o ${artifact.name}.zip -d artifact_files/${artifact.name}
                  rm ${artifact.name}.zip
                  """
                } catch (err) {
                  echo "Failed to download artifact ${artifact.name}: ${err}"
                  error("Artifact download failed.")
                }
              }
            }

          } 
          // Windows version
          else {
            bat """
            curl -s -H "Authorization: token ${GITHUB_TOKEN}" ^
              https://api.github.com/repos/${GITHUB_OWNER}/${GITHUB_REPO}/actions/runs/${params.RUN_ID}/artifacts ^
              > artifacts.json
            """

            def artifactsJson = readFile('artifacts.json').trim()
            def parsed = readJSON text: artifactsJson
            def artifacts = parsed.artifacts

            for (artifact in artifacts) {
              if (artifact.name.contains(params.BATCH_ID)) {
                echo "Downloading artifact: ${artifact.name} (ID: ${artifact.id})"
                env.ARTIFACT_NAME = artifact.name

                try {
                  bat """
                  mkdir artifact_files\\${artifact.name}

                  curl -L -H "Authorization: token ${GITHUB_TOKEN}" ^
                    -o ${artifact.name}.zip ^
                    https://api.github.com/repos/${GITHUB_OWNER}/${GITHUB_REPO}/actions/artifacts/${artifact.id}/zip

                  powershell -Command "Expand-Archive -Path '${artifact.name}.zip' -DestinationPath 'artifact_files/${artifact.name}' -Force"
                  del ${artifact.name}.zip
                  """
                } catch (err) {
                  echo "Failed to download artifact ${artifact.name}: ${err}"
                  error("Artifact download failed.")
                }
              }
            }
          }
        }
      }
    }

    stage('Prepare Flash Tools') {
      steps {
        script {
          // Download and extract the entire dev branch zip
          if (isUnix()) {
            sh '''
              curl -L -o dev.zip https://github.com/Ameba-AIoT/ameba-arduino-pro2/archive/refs/heads/dev.zip
              unzip -o dev.zip
            '''
          } else {
            bat '''
              curl -L -o dev.zip https://github.com/Ameba-AIoT/ameba-arduino-pro2/archive/refs/heads/dev.zip
              tar -xf dev.zip
            '''
          }

          // Create the target tools folder then copy the extracted platform-specific tools into that folder
          if (isUnix()) {
            sh "mkdir -p ${TOOLS_FOLDER}"
            sh "cp -r ${TOOLS_SOURCE}/* ${TOOLS_FOLDER}/"
          } else {
            bat "mkdir ${TOOLS_FOLDER}"
            powershell """
              Copy-Item -Recurse -Force '${TOOLS_SOURCE}\\*' '${TOOLS_FOLDER}\\'
            """
          }
          echo "Build tools copied to ${TOOLS_FOLDER}"
        }
      }
    }

    stage('Flash to Hardware') {
      when {
        expression { params.RUN_ID && params.BATCH_ID }
      }
      steps {
        lock(resource: 'test-board', quantity: 1) {
          script {
            def comPort = ""
            def pythonExe = ""
            // Find serial port for Linux or macOS
            if (isUnix()) {
              echo "Detecting serial port..."
              def serialList = sh(
                script: '''
                  ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null || true
                ''',
                returnStdout: true
              ).trim().split('\n')
              if (serialList.size() == 0 || serialList[0].trim() == "") {
                error("No serial ports found! Is the board connected?")
              }
              comPort = serialList[0].trim()
              echo "Detected serial port: ${comPort}"
              pythonExe = "python3"

              sh """
                ${pythonExe} scripts/flash_firmware.py \
                  --image_exe "${IMAGE_EXE}" \
                  --tools_path "${TOOLS_FOLDER}" \
                  --com_port "${comPort}" \
                  --board "${board}"
              """
            } 
            // Windows
            else {
              def comPorts = powershell(returnStdout: true, script: '''
              Get-PnpDevice -Class "Ports" | Where-Object { $_.FriendlyName -match "COM" } | ForEach-Object {
                if ($_ -match "\\(COM[0-9]+\\)") {
                  $matches[0] -replace "[()]", ""
                }
              }
              ''').trim().split('\n')
              
              // comport = comPorts[0].trim()
              comPort = "COM3" 
              echo "Detected COM port: ${comPort}"

              def userNameRaw = bat(script: 'echo %USERNAME%', returnStdout: true).trim()
              def userName = userNameRaw.split('\n')[-1].trim()
              echo "Agent user: ${userName}"

              pythonExe = "C:\\Users\\${userName}\\AppData\\Local\\Programs\\Python\\Python313\\python.exe"

              bat """
                ${pythonExe} scripts\\flash_firmware.py ^
                  --image_exe "${IMAGE_EXE}" ^
                  --tools_path "${TOOLS_FOLDER}" ^
                  --com_port "${comPort}" ^
                  --board "${board}"
              """
            }
          }
        }
      }
      post {
        always {
          echo "Job done for batch ${params.BATCH_ID}"
        }
      }
    }
  }
}