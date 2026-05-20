pipeline {
    agent any

    environment {
        SIM_IMAGE  = "satellite-sim:${BUILD_NUMBER}"
        PROC_IMAGE = "satellite-proc:${BUILD_NUMBER}"
    }

    stages {
        stage('Build C++') {
            steps {
                sh '''
                    cmake -B build -S . \
                        -DCMAKE_BUILD_TYPE=Release \
                        -DCMAKE_POLICY_DEFAULT_CMP0135=NEW
                    cmake --build build -j$(nproc)
                    ctest --test-dir build --output-on-failure -V
                '''
            }
            post {
                always {
                    junit 'build/**/*.xml'
                }
            }
        }

        stage('Python Tests') {
            steps {
                sh '''
                    pip install uv --quiet
                    uv sync --frozen
                    uv run pytest tests_python/ -v --tb=short \
                        --junitxml=pytest-results.xml
                '''
            }
            post {
                always {
                    junit 'pytest-results.xml'
                }
            }
        }

        stage('Docker Build') {
            steps {
                sh '''
                    docker build -f Dockerfile.sim  -t ${SIM_IMAGE}  .
                    docker build -f Dockerfile.proc -t ${PROC_IMAGE} .
                '''
            }
        }

        stage('Smoke Test') {
            steps {
                sh '''
                    bash scripts/setup.sh
                    SIM_IMAGE=${SIM_IMAGE} PROC_IMAGE=${PROC_IMAGE} \
                        docker compose up -d

                    # Wait up to 30s for processor health endpoint
                    for i in $(seq 1 30); do
                        if curl -sf http://localhost:8091/api/health > /dev/null 2>&1; then
                            echo "Processor healthy after ${i}s"
                            break
                        fi
                        sleep 1
                    done

                    curl -sf http://localhost:8091/api/health | \
                        python3 -c "import sys, json; d=json.load(sys.stdin); sys.exit(0 if d['status']=='ok' else 1)"

                    docker compose down
                '''
            }
        }
    }

    post {
        always {
            sh 'docker compose down --remove-orphans || true'
            archiveArtifacts artifacts: 'build/*.xml, pytest-results.xml', allowEmptyArchive: true
        }
        failure {
            echo 'Pipeline failed — check stage output above'
        }
    }
}
