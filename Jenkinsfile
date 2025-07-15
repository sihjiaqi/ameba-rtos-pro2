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

    stage ('Setup OS Paths') {
      steps {
        script {
          def toolsFolder = ""
          def imageExe = ""
          if (isUnix()) {
            def unameOut = sh(script: 'uname', returnStdout: true).trim()
            if (unameOut == "Darwin") {
              echo "Detected macOS"
              toolsFolder = "${env.WORKSPACE}/unzipped_artifacts/ameba_pro2_tools_macos"
              imageExe    = "${toolsFolder}/image_macos"
            } else {
              echo "Detected Linux"
              toolsFolder = "${env.WORKSPACE}/unzipped_artifacts/ameba_pro2_tools_linux"
              imageExe    = "${toolsFolder}/image_linux"
            }
          } else {
            echo "Detected Windows"
            toolsFolder = "${env.WORKSPACE}\\unzipped_artifacts\\ameba_pro2_tools_windows"
            imageExe    = "${toolsFolder}\\image_windows.exe"
          }
          env.TOOLS_FOLDER = toolsFolder
          env.IMAGE_EXE = imageExe
        }
      }
    }

    stage('Download Firmware Artifacts') {
      steps {
        script {
          echo "Run ID: ${params.RUN_ID} | Batch ID: ${params.BATCH_ID}"
          // Curl command to fetch artifacts
          bat """
          curl -s -H "Authorization: token ${GITHUB_TOKEN}" ^
          https://api.github.com/repos/${GITHUB_OWNER}/${GITHUB_REPO}/actions/runs/${params.RUN_ID}/artifacts ^
          > artifacts.json
          """

          def artifactsJson = readFile('artifacts.json').trim() // Read the JSON response from the file
          def parsed = readJSON text: artifactsJson // Parse the JSON response to a Groovy object
          def artifacts = parsed.artifacts // Extract the artifacts array

          // Download artifact with the specified batch ID
          for (artifact in artifacts) {
            if (artifact.name.contains(params.BATCH_ID)) {
              found = true
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

    stage('Prepare Flash Tools') {
      steps {
        script {
          def toolsSource = "ameba-arduino-pro2-dev\\Arduino_package\\ameba_pro2_tools_windows"

          // Download and extract entire dev branch zip
          bat '''
            curl -L -o dev.zip https://github.com/Ameba-AIoT/ameba-arduino-pro2/archive/refs/heads/dev.zip
            tar -xf dev.zip
          '''
          // Create target tools folder if missing
          bat "mkdir ${TOOLS_FOLDER}"
          // Copy only the tools folder content
          powershell """
            Copy-Item -Recurse -Force '${toolsSource}\\*' '${TOOLS_FOLDER}\\'
          """
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
            // Detect COM port on Windows
            def comPorts = powershell(returnStdout: true, script: '''
            Get-PnpDevice -Class "Ports" | Where-Object { $_.FriendlyName -match "COM" } | ForEach-Object {
              if ($_ -match "\\(COM[0-9]+\\)") {
                $matches[0] -replace "[()]", ""
              }
            }
            ''').trim().split('\n')

            // def comPort = comPorts[0].trim()  // pick first COM port only
            def comPort = "COM3"
            echo "Detected COM port: ${comPort}"
            def userNameRaw = bat(script: 'echo %USERNAME%', returnStdout: true).trim()
            def userName = userNameRaw.split('\n')[-1].trim()
            echo "Agent user: ${userName}"
            def pythonExe = "C:\\Users\\${userName}\\AppData\\Local\\Programs\\Python\\Python313\\python.exe"
            
            bat """
              ${pythonExe} scripts\\flash_firmware.py ^
                --image_exe "${IMAGE_EXE}" ^
                --tools_path "${TOOLS_FOLDER}" ^
                --com_port "${comPort}" ^
                --board "Ameba_AMB82-MINI"
            """
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