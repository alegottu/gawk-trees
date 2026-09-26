pipeline {
	agent any
	stages {
		stage('build') {
			when {
				changeset "tools/convert/convert.py"
			}
			steps {
				dir("tools/convert") {
					sh 'python3 test_convert.py | tee "convert_test_`date +%m-%d-%y`.txt"'
				}
			}
		}
	}
	post {
		success {
			sh 'echo "removing conversion script unit test logs"'
			sh 'rm -f tools/convert/*test*.txt'
		}
		failure {
			sh 'echo "archiving conversion script unit test logs"'
			archiveArtifacts 'tools/convert/*test*.txt'
		}
	}
}
