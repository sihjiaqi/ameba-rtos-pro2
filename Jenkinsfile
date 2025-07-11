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

          // Download the artifact with the specified batch ID
          for (artifact in artifacts) {
            if (artifact.name.contains(params.BATCH_ID)) {
              found = true
              echo "Downloading artifact: ${artifact.name} (ID: ${artifact.id})"
              env.ARTIFACT_NAME = artifact.name

              try {
                bat """
                mkdir artifact_files\\${artifact.name}

                # Download artifact
                curl -L -H "Authorization: token ${GITHUB_TOKEN}" ^
                  -o ${artifact.name}.zip ^
                  https://api.github.com/repos/${GITHUB_OWNER}/${GITHUB_REPO}/actions/artifacts/${artifact.id}/zip
                  
                # Unzip the downloaded artifact
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

    stage('Prepare Arduino Tools') {
      steps {
        script {
          def toolsFolder = "${env.WORKSPACE}/unzipped_artifacts/ameba_pro2_tools_linux"
          def imageToolFolder = "${toolsFolder}/image_tool"
          def imageLinuxFile = "${toolsFolder}/image_linux"

          // Check if folder and file exist
          def imageToolExists = fileExists(imageToolFolder)
          def imageLinuxExists = fileExists(imageLinuxFile)

          if (!imageToolExists || !imageLinuxExists) {
            echo "Required Arduino tools missing, cloning repo and copying files..."

            // Clone repo
            sh 'rm -rf ameba-arduino-pro2'
            sh 'git clone https://github.com/Ameba-AIoT/ameba-arduino-pro2.git'

            // Create target folders if missing
            sh "mkdir -p ${imageToolFolder}"

            // Copy folder and file to unzipped_artifacts folder
            sh """
            cp -r ameba-arduino-pro2/Arduino_package/ameba_pro2_tools_linux/image_tool/* ${imageToolFolder}/
            cp ameba-arduino-pro2/Arduino_package/ameba_pro2_tools_linux/image_linux ${toolsFolder}/
            """
          } else {
            echo "Arduino tools already present, skipping clone."
          }
        }
      }
    }

    stage('Flash to Hardware') {
      when {
        expression { params.RUN_ID && params.BATCH_ID }
      }
      steps {
        lock(label: 'test-board', quantity: 1) {
          script {
            // Detect COM port on Windows
            def comPort = bat(returnStdout: true, script: 'powershell -Command "Get-WmiObject Win32_SerialPort | Select-Object -ExpandProperty DeviceID"').trim()
            echo "Detected COM port: ${comPort}"

            def board = "Ameba_AMB82-MINI"
            def toolsPath = ""
            def imageExePath = ""
            def artifactName = env.ARTIFACT_NAME ?: ""
            if (artifactName.toLowerCase().contains("ubuntu")) {
                toolsPath = "${env.WORKSPACE}/unzipped_artifacts/ameba_pro2_tools_linux"
                imageExePath = "${toolsPath}/image_linux.exe"
            } else if (artifactName.toLowerCase().contains("macos")) {
                toolsPath = "${env.WORKSPACE}/unzipped_artifacts/ameba_pro2_tools_macos"
                imageExePath = "${toolsPath}/image_macos.exe"
            } else if (artifactName.toLowerCase().contains("windows")) {
                toolsPath = "${env.WORKSPACE}/unzipped_artifacts/ameba_pro2_tools_windows"
                imageExePath = "${toolsPath}/image_windows.exe"
            } else {
                error("Unknown artifact name: cannot determine OS tools to use.")
            }

            bat """
            python flash_firmware.py ^
                --image_exe "${imageExePath}" ^
                --tools_path "${toolsPath}" ^
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