pipeline {
  agent { label 'windows-agent' }

  parameters {
    string(name: 'RUN_ID', defaultValue: '', description: 'GitHub Actions run ID')
    string(name: 'BATCH_ID', defaultValue: '', description: 'Which batch')
  }

  environment {
    GITHUB_OWNER = 'sihjiaqi'
    GITHUB_REPO  = 'ameba-rtos-pro2'
    GITHUB_TOKEN = credentials('GITHUB_TOKEN')  // Store GitHub PAT in Jenkins Credentials
  }

  stages {
    stage('Test Token') {
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
    
    stage('Debug Parameters') {
      steps {
        script {
          echo "RUN_ID = ${params.RUN_ID}"
          echo "BATCH_ID = ${params.BATCH_ID}"
        }
      }
    }

    stage('Get Firmware from GitHub Artifacts') {
      steps {
        script {
          echo "Getting artifacts for run ID: ${params.RUN_ID}, batch ID: ${params.BATCH_ID}"

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
              found = true
              echo "Downloading artifact: ${artifact.name} (ID: ${artifact.id})"
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
                error("Artifact download failed!")
              }
            }
          }
        }
      }
    }
  }
}