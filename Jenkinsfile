pipeline {
	agent any
	stages {
		stage('build-extension') {
			when {
				changeset "src/*"
			}
			steps {
				sh 'rm -f bin/*'
				sh 'make debug 2>&1 > buildg.out'
				sh 'make verbose 2>&1 > buildv.out'

				sh 'rm test/test'
				sh 'make test 2>&1 > buildt.out'
			}
			post {
				failure {
					archiveArtifacts 'build*.out'
				}
				always {
					sh 'rm -f build*.out'
				}
			}
		}
		stage('test-extension') {
			when {
				changeset "src/*"
			}
			steps {
				dir ("test") {
					sh 'gawk -lghtrees -f test.awk <<< "1000 1000" 2>&1 | tee awktest.out'
					sh './gen-data.sh 1000 3 8 | gawk -f test-rand.awk 2>&1 | tee randtest.out'
					sh './test 1000 2>&1 | tee ctest.out'
				}
			}
			post {
				failure {
					archiveArtifacts 'test/*test.out'
				}
				always {
					sh 'rm -f test/*test.out'
				}
			}
		}
		stage('profile-extension') {
			when {
				changeset "src/*"
			}
			steps {
				dir ("tools/benchmark") {
					sh 'py mem_benchmark.py -i 100000-10x2 2>&1 > /dev/null'
				}
			}
			post {
				success {
					archiveArtifacts 'tools/benchmark/*.data'
				}
				always {
					sh 'rm -f tools/benchmark/*.data'
				}
			}
		}
		stage('test-libwayne') {
			when {
				changeset "libwayne/src/*"
			}
			steps {
				dir ("test") {
					sh 'gawk -f avl-sanity-test.awk <<< "512 512 8" 2>&1 > /dev/null | tee avltest.out'
				}
			}
			post {
				failure {
					archiveArtifacts 'test/*test.out'
				}
				always {
					sh 'rm -f test/*test.out'
				}
			}
		}
		stage('test-convert') {
			when {
				changeset "tools/convert/convert.py"
			}
			steps {
				dir("tools/convert") {
					sh 'python3 test_convert.py | tee "convert_test_`date +%m-%d-%y`.txt"'
				}
			}
			post {
				failure {
					archiveArtifacts 'tools/convert/*test*.txt'
				}
				always {
					sh 'rm -f tools/convert/*test*.txt'
				}
			}
		}
	}
}
