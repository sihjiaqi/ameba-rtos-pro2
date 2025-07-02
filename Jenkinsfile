pipeline {
  agent any

  parameters {
    string(name: 'RUN_ID', defaultValue: '', description: 'GitHub Actions run ID')
  }

  environment {
    GITHUB_OWNER = 'sihjiaqi'
    GITHUB_REPO  = 'ameba-rtos-pro2'
    GITHUB_TOKEN = credentials('GITHUB_TOKEN')  // Store GitHub PAT in Jenkins Credentials
  }

  stages {
    stage('Get Firmware from GitHub Artifacts') {
        steps {
            script {
                echo "Getting artifacts for run ID: ${params.RUN_ID}"

                def artifactsJson = sh (
                    script: """
                    curl -s -H "Authorization: token ${GITHUB_TOKEN}" \
                    https://api.github.com/repos/${GITHUB_OWNER}/${GITHUB_REPO}/actions/runs/${params.RUN_ID}/artifacts
                    """,
                    returnStdout: true
                ).trim()

                def parsed = readJSON text: artifactsJson
                def artifacts = parsed.artifacts

                for (artifact in artifacts) {
                    echo "Downloading artifact: ${artifact.name} (ID: ${artifact.id})"

                    sh """
                    curl -L -H "Authorization: token ${GITHUB_TOKEN}" \
                        -o ${artifact.name}.zip \
                        https://api.github.com/repos/${GITHUB_OWNER}/${GITHUB_REPO}/actions/artifacts/${artifact.id}/zip

                    unzip -o ${artifact.name}.zip -d artifact_files/${artifact.name}
                    """
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