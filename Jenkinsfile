pipeline {
  agent { label 'windows-agent' }

  parameters {
    string(name: 'GERRIT_PROJECT', defaultValue: '', description: 'Gerrit project name (from Gerrit Trigger)')
    string(name: 'GERRIT_BRANCH', defaultValue: 'main', description: 'Branch to build (from Gerrit Trigger)')
  }

  environment {
    GERRIT_CREDENTIAL_ID = 'gerrit-ssh-key-credential-id'
    BOARD = 'Ameba_AMB82-MINI'
  }

  stages {
    stage('Clean Workspace') {
      steps {
        cleanWs()
      }
    }

    stage('Checkout RTOS Repo from Gerrit') {
      steps {
        checkout([$class: 'GitSCM',
          branches: [[name: "${params.GERRIT_BRANCH}"]],
          doGenerateSubmoduleConfigurations: false,
          extensions: [],
          userRemoteConfigs: [[
            url: "ssh://sihjiaqi@sgcn3sd3-git.rtkbf.com:29418/${params.GERRIT_PROJECT}",
            credentialsId: "${env.GERRIT_CREDENTIAL_ID}"
          ]]
        ])
      }
    }

    stage('Checkout Arduino Tools Repo') {
      steps {
        dir('ameba-arduino-pro2-forked') {
          checkout([$class: 'GitSCM',
            branches: [[name: "main"]],
            userRemoteConfigs: [[
              url: "ssh://sihjiaqi@sgcn3sd3-git.rtkbf.com:29418/ameba-arduino-pro2-forked",
              credentialsId: "${env.GERRIT_CREDENTIAL_ID}"
            ]]
          ])
        }
      }
    }

    stage('Pull LFS binaries') {
      steps {
        sh 'git lfs pull'
      }
    }

    stage('Setup Tools Folder') {
      steps {
        script {
          def toolsFolder = ""
          def imageExe = ""
          def toolsSource = ""

          echo "Detected OS: ${isUnix() ? 'Unix-like' : 'Windows'}"

          if (isUnix()) {
            toolsSource = "ameba-arduino-pro2-forked/Arduino_package/ameba_pro2_tools_linux"
            toolsFolder = "${env.WORKSPACE}/unzipped_artifacts/ameba_pro2_tools_linux"
            imageExe    = "${toolsFolder}/image_linux"
            sh "mkdir -p ${toolsFolder}"
            sh "cp -r ${toolsSource}/* ${toolsFolder}/"
          } else {
            toolsSource = "ameba-arduino-pro2-forked\\Arduino_package\\ameba_pro2_tools_windows"
            toolsFolder = "${env.WORKSPACE}\\unzipped_artifacts\\ameba_pro2_tools_windows"
            imageExe    = "${toolsFolder}\\image_windows.exe"
            bat "mkdir ${toolsFolder}"
            powershell """
              Copy-Item -Recurse -Force '${toolsSource}\\*' '${toolsFolder}\\'
            """
          }

          env.TOOLS_FOLDER = toolsFolder
          env.IMAGE_EXE = imageExe

          echo "Tools prepared in: ${toolsFolder}"
        }
      }
    }

    stage('Prepare Flash Tools') {
      steps {
        script {
          if (isUnix()) {
            sh "mkdir -p ${TOOLS_FOLDER}"
            sh "cp -r arduino-tools-repo/Arduino_package/ameba_pro2_tools_linux/* ${TOOLS_FOLDER}/"
          } else {
            bat "mkdir ${TOOLS_FOLDER}"
            bat "xcopy arduino-tools-repo\\Arduino_package\\ameba_pro2_tools_windows\\* ${TOOLS_FOLDER}\\ /E /I /Y"
          }
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
            def scriptPath = ""
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
              scriptPath = "${env.WORKSPACE}/scripts/flash_firmware.py"
              sh """
                ${pythonExe} ${scriptPath} \
                  --image_exe "${IMAGE_EXE}" \
                  --tools_path "${TOOLS_FOLDER}" \
                  --com_port "${comPort}" \
                  --board "${board}"
                  > flash_log.txt 2>&1
              """
              sh "cat flash_log.txt"
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
              scriptPath = "${env.WORKSPACE}\\scripts\\flash_firmware.py"
              bat """
                ${pythonExe} ${scriptPath} ^
                  --image_exe "${IMAGE_EXE}" ^
                  --tools_path "${TOOLS_FOLDER}" ^
                  --com_port "${comPort}" ^
                  --board "${board}" > flash_log.txt 2>&1
              """
              bat "type flash_log.txt"
            }
          }
        }
      }
      post {
        always {
          archiveArtifacts artifacts: 'flash_log.txt', allowEmptyArchive: true
          echo "Job done for batch ${params.BATCH_ID}"
        }
      }
    }
  }
}