pipeline {
  agent { label 'windows-agent' }

  parameters {
    string(name: 'RUN_ID', defaultValue: '', description: 'GitHub Actions run ID')
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

    stage('Get Firmware from GitHub Artifacts') {
      steps {
        script {
          echo "Getting artifacts for run ID: ${params.RUN_ID}"

          bat """
          curl -s -H "Authorization: token ${GITHUB_TOKEN}" ^
          https://api.github.com/repos/${GITHUB_OWNER}/${GITHUB_REPO}/actions/runs/${params.RUN_ID}/artifacts ^
          > artifacts.json
          """

          def artifactsJson = readFile('artifacts.json').trim()
          echo "Artifacts JSON: ${artifactsJson}"

          def parsed = readJSON text: artifactsJson
          def artifacts = parsed.artifacts

          for (artifact in artifacts) {
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
            }
          }
        }
      }
    }

    // stage('Run Hardware Test') {
    //   steps {
    //     script {
    //       echo "Running test with files in artifact_files/"
    //       // Add your commands here: e.g. scp to test board, call test script, etc.
    //     }
    //   }
    // }
  }
}